/****************************************************************************
 * fs/fat/fs_fat32.c
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/mount.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <debug.h>
#include <time.h>

#include <nuttx/kmalloc.h>
#include <nuttx/fs/fs.h>

#include "inode/inode.h"

#include "ff.h"

/****************************************************************************
 * Private Types
 ****************************************************************************/

#define FATATTR_VOLUMEID 0x08	/* Volume label */
#define MSDOS_SUPER_MAGIC 0x4d44

struct inode *regist_inode[FF_VOLUMES] = { 0 };

struct fat_mountpt_s
{
    FATFS *ff_fs;
    FAR struct inode *fs_blkdriver;
    char name[NAME_MAX + 1];
    mutex_t fs_lock;
};

/****************************************************************************
 * Private Function Prototypes
 ****************************************************************************/

static int     fat_open(FAR struct file *filep, FAR const char *relpath,
                 int oflags, mode_t mode);
static int     fat_close(FAR struct file *filep);
static ssize_t fat_read(FAR struct file *filep, FAR char *buffer,
                 size_t buflen);
static ssize_t fat_write(FAR struct file *filep, FAR const char *buffer,
                 size_t buflen);
static off_t   fat_seek(FAR struct file *filep, off_t offset, int whence);
static int     fat_ioctl(FAR struct file *filep, int cmd,
                 unsigned long arg);

static int     fat_sync(FAR struct file *filep);
static int     fat_dup(FAR const struct file *oldp, FAR struct file *newp);
static int     fat_fstat(FAR const struct file *filep,
                 FAR struct stat *buf);
static int     fat_truncate(FAR struct file *filep, off_t length);

static int     fat_opendir(FAR struct inode *mountpt,
                 FAR const char *relpath, FAR struct fs_dirent_s **dir);
static int     fat_closedir(FAR struct inode *mountpt,
                 FAR struct fs_dirent_s *dir);
static int     fat_readdir(FAR struct inode *mountpt,
                 FAR struct fs_dirent_s *dir,
                 FAR struct dirent *entry);
static int     fat_rewinddir(FAR struct inode *mountpt,
                 FAR struct fs_dirent_s *dir);

static int     fat_bind(FAR struct inode *blkdriver, FAR const void *data,
                 FAR void **handle);
static int     fat_unbind(FAR void *handle,
                 FAR struct inode **blkdriver, unsigned int flags);
static int     fat_statfs(FAR struct inode *mountpt,
                 FAR struct statfs *buf);

static int     fat_unlink(FAR struct inode *mountpt,
                 FAR const char *relpath);
static int     fat_mkdir(FAR struct inode *mountpt, FAR const char *relpath,
                 mode_t mode);
static int     fat_rmdir(FAR struct inode *mountpt, FAR const char *relpath);
static int     fat_rename(FAR struct inode *mountpt,
                 FAR const char *oldrelpath, FAR const char *newrelpath);
static int     fat_stat_common(FAR struct fat_mountpt_s *fs,
                 FILINFO *file_info, FAR struct stat *buf);
static int     fat_stat_file(FAR struct fat_mountpt_s *fs,
                 FILINFO *file_info, FAR struct stat *buf);
static int     fat_stat(struct inode *mountpt, const char *relpath,
                 FAR struct stat *buf);

/****************************************************************************
 * Public Data
 ****************************************************************************/

/* See fs_mount.c -- this structure is explicitly extern'ed there.
 * We use the old-fashioned kind of initializers so that this will compile
 * with any compiler.
 */

const struct mountpt_operations g_fat_operations = {
    fat_open,          /* open */
    fat_close,         /* close */
    fat_read,          /* read */
    fat_write,         /* write */
    fat_seek,          /* seek */
    fat_ioctl,         /* ioctl */
    NULL,              /* mmap */
    fat_truncate,      /* truncate */
    fat_sync,          /* sync */
    fat_dup,           /* dup */
    fat_fstat,         /* fstat */
    NULL,              /* fchstat */

    fat_opendir,       /* opendir */
    fat_closedir,      /* closedir */
    fat_readdir,       /* readdir */
    fat_rewinddir,     /* rewinddir */

    fat_bind,          /* bind */
    fat_unbind,        /* unbind */
    fat_statfs,        /* statfs */

    fat_unlink,        /* unlink */
    fat_mkdir,         /* mkdir */
    fat_rmdir,         /* rmdir */
    fat_rename,        /* rename */
    fat_stat,          /* stat */
    NULL               /* chstat */
};

/****************************************************************************
 * Private Functions
 ****************************************************************************/
static BYTE make_ff_mode(int oflags)
{
    BYTE mode = 0;
    static const int modeflags[][2] = {
        {O_RDONLY, FA_READ},
        {O_RDWR, FA_READ | FA_WRITE},
        {O_WRONLY | O_CREAT | O_TRUNC, FA_WRITE | FA_CREATE_ALWAYS},
        {O_RDWR | O_CREAT | O_TRUNC, FA_READ | FA_WRITE | FA_CREATE_ALWAYS},
        {O_WRONLY | O_CREAT | O_APPEND, FA_WRITE | FA_CREATE_NEW | FA_OPEN_APPEND},
        {O_RDWR | O_CREAT | O_APPEND, FA_READ | FA_WRITE | FA_CREATE_NEW | FA_OPEN_APPEND},
        {0, 0},
    };

    for(int i = 0; modeflags[i][0] != sizeof(modeflags)/sizeof(*modeflags); i++) {
        if (modeflags[i][0] == oflags) {
            mode |= modeflags[i][1];
            return mode;
        }
    }

    return mode;
}

/****************************************************************************
 * Name: fat_open
 ****************************************************************************/

static int fat_open(FAR struct file *filep, FAR const char *relpath,
                    int oflags, mode_t mode)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    FIL *ff;
    int ret;
    BYTE ff_mode = 0;

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv == NULL && filep->f_inode != NULL);

    /* Get the mountpoint inode reference from the file structure and the
    * mountpoint private data from the inode structure
    */

    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ff = (FIL *)kmm_zalloc(sizeof(FIL));
    if (!ff) {
        ret = -ENOMEM;
        goto errout_with_lock;
    }
    ff_mode = make_ff_mode(oflags);
    if (ff_mode == 0) {
        ret = -ENODEV;
        goto errout_with_struct;
    }
    ret = f_open(ff, relpath, ff_mode);
    if (ret != FR_OK) {
        ret = -ENODEV;
        goto errout_with_struct;
    }
    /* Attach the private date to the struct file instance */
    filep->f_priv = ff;

    nxmutex_unlock(&fs->fs_lock);

    /* In write/append mode, we need to set the file pointer to the end of
    * the file.
    */

    if ((oflags & (O_APPEND | O_WRONLY)) == (O_APPEND | O_WRONLY)) {
        off_t offset = fat_seek(filep, ff->obj.objsize, SEEK_SET);
        if (offset < 0) {
            kmm_free(ff);
            return (int)offset;
        }
    }

    return OK;

/* Error exits -- goto's are nasty things, but they sure can make error
* handling a lot simpler.
*/
errout_with_struct:
    kmm_free(ff);

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_close
 ****************************************************************************/

static int fat_close(FAR struct file *filep)
{
    FAR struct inode *inode;
    FIL *ff;
    FAR struct fat_mountpt_s *fs;
    int ret = OK;

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Recover our private data from the struct file instance */

    ff    = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    ret = f_close(ff);

    kmm_free(ff);
    filep->f_priv = NULL;
    return ret;
}

/****************************************************************************
 * Name: fat_read
 ****************************************************************************/

static ssize_t fat_read(FAR struct file *filep, FAR char *buffer,
                        size_t buflen)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    FIL *ff;
    int ret;
    UINT read_size = 0;

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Recover our private data from the struct file instance */

    ff = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    ret = f_read(ff, buffer, buflen, &read_size);
    if (ret != FR_OK) {
        goto errout_with_lock;
    }
    nxmutex_unlock(&fs->fs_lock);
    return (ssize_t)read_size;

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_write
 ****************************************************************************/

static ssize_t fat_write(FAR struct file *filep, FAR const char *buffer,
                         size_t buflen)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    FIL *ff;
    int ret;
    UINT write_size = 0;

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Recover our private data from the struct file instance */

    ff = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Make sure that the mount is still healthy */
    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    ret = f_write(ff, buffer, buflen, &write_size);
    if (ret != FR_OK) {
        goto errout_with_lock;
    }
    nxmutex_unlock(&fs->fs_lock);
    return (ssize_t)write_size;

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_seek
 ****************************************************************************/

static off_t fat_seek(FAR struct file *filep, off_t offset, int whence)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    FIL *ff;
    int ret;
    FSIZE_t ofs = 0;

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Recover our private data from the struct file instance */
    ff = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);
    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    if (whence == SEEK_CUR) {
        ofs = ff->fptr;
    }
    if (whence == SEEK_END) {
        ofs = ff->obj.objsize;
    }
    ofs += offset;
    ret = f_lseek(ff, ofs);
    if (ret != FR_OK) {
        ret = -EINVAL;
        goto errout_with_lock;
    }

    nxmutex_unlock(&fs->fs_lock);
    return OK;

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_ioctl
 ****************************************************************************/

static int fat_ioctl(FAR struct file *filep, int cmd, unsigned long arg)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Check for the forced mount condition */

    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Make sure that the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    /* ioctl calls are just passed through to the contained block driver */

    nxmutex_unlock(&fs->fs_lock);
    return -ENOSYS;
}

/****************************************************************************
 * Name: fat_sync
 *
 * Description: Synchronize the file state on disk to match internal, in-
 *   memory state.
 *
 ****************************************************************************/

static int fat_sync(FAR struct file *filep)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    FIL *ff;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Check for the forced mount condition */

    ff = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Make sure that the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    ret = f_sync(ff);
    if (ret != FR_OK) {
        ret = -EINVAL;
    }
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_dup
 *
 * Description: Duplicate open file data in the new file structure.
 *
 ****************************************************************************/

static int fat_dup(FAR const struct file *oldp, FAR struct file *newp)
{
    FAR struct fat_mountpt_s *fs;
    FIL *oldff;
    FIL *newff;
    int ret;

    finfo("Dup %p->%p\n", oldp, newp);

    /* Sanity checks */

    DEBUGASSERT(oldp->f_priv != NULL &&
                newp->f_priv == NULL &&
                newp->f_inode != NULL);

    /* Recover the old private data from the old struct file instance */

    oldff = oldp->f_priv;
    fs = (struct fat_mountpt_s *)oldp->f_inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    newff = (FIL *)kmm_malloc(sizeof(FIL));
    if (!newff) {
        ret = -ENOMEM;
        goto errout_with_lock;
    }

    newff->obj = oldff->obj;
    newff->flag = oldff->flag;
    newff->err = oldff->err;
    newff->fptr = oldff->fptr;
    newff->clust = oldff->clust;
    newff->sect = oldff->sect;
#if !FF_FS_READONLY
    newff->dir_sect = oldff->dir_sect;
    newff->dir_ptr = oldff->dir_ptr;
#endif
#if FF_USE_FASTSEEK
    newff->cltbl = oldff->cltbl;
#endif
#if !FF_FS_TINY
    if (memcpy(newff->buf, oldff->buf, FF_MAX_SS) != newff->buf) {
        ret = -ENOMEM;
        goto errout_with_struct;
    }
#endif

    /* Attach the private date to the struct file instance */

    newp->f_priv = newff;

    /* Then insert the new instance into the mountpoint structure.
    * It needs to be there (1) to handle error conditions that effect
    * all files, and (2) to inform the umount logic that we are busy
    * (but a simple reference count could have done that).
    */

    nxmutex_unlock(&fs->fs_lock);
    return OK;

    /* Error exits -- goto's are nasty things, but they sure can make error
    * handling a lot simpler.
    */

errout_with_struct:
    kmm_free(newff);

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_opendir
 *
 * Description: Open a directory for read access
 *
 ****************************************************************************/

static int fat_opendir(FAR struct inode *mountpt, FAR const char *relpath,
                       FAR struct fs_dirent_s **dir)
{
    FF_DIR *fdir;
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(mountpt != NULL && mountpt->i_private != NULL);

    /* Recover our private data from the inode instance */

    fs = mountpt->i_private;

    fdir = kmm_zalloc(sizeof(FF_DIR));
    if (fdir == NULL) {
        return -ENOMEM;
    }

    /* Make sure that the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        goto errout_with_fdir;
    }

    ret = f_opendir(fdir, relpath);
    if (ret != FR_OK) {
        ret = -EINVAL;
        goto errout_with_lock;
    }

    *dir = (FAR struct fs_dirent_s *)fdir;
    nxmutex_unlock(&fs->fs_lock);
    return OK;

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);

errout_with_fdir:
    kmm_free(fdir);
    return ret;
}

/****************************************************************************
 * Name: fat_closedir
 *
 * Description: Close directory
 *
 ****************************************************************************/

static int fat_closedir(FAR struct inode *mountpt,
                        FAR struct fs_dirent_s *dir)
{
    DEBUGASSERT(dir);
    f_closedir((FF_DIR*)dir);
    kmm_free(dir);
    return 0;
}

/****************************************************************************
 * Name: fat_fstat
 *
 * Description:
 *   Obtain information about an open file associated with the file
 *   structure 'filep', and will write it to the area pointed to by 'buf'.
 *
 ****************************************************************************/

static int fat_fstat(FAR const struct file *filep, FAR struct stat *buf)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    int ret;
    FILINFO file_info = { 0 };

    /* Sanity checks */

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Get the mountpoint inode reference from the file structure and the
    * mountpoint private data from the inode structure
    */

    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    /* Recover our private data from the struct file instance */

    ret = f_stat(fs->name, &file_info);
    if (ret != FR_OK) {
        goto errout_with_lock;
    }

    ret = fat_stat_file(fs, &file_info, buf);

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_truncate
 *
 * Description:
 *   Set the length of the open, regular file associated with the file
 *   structure 'filep' to 'length'.
 *
 ****************************************************************************/

static int fat_truncate(FAR struct file *filep, off_t length)
{
    FAR struct inode *inode;
    FAR struct fat_mountpt_s *fs;
    FIL *ff;
    int ret;

    DEBUGASSERT(filep->f_priv != NULL && filep->f_inode != NULL);

    /* Recover our private data from the struct file instance */

    ff = filep->f_priv;
    inode = filep->f_inode;
    fs    = inode->i_private;

    DEBUGASSERT(fs != NULL);

    /* Make sure that the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    ff->fptr = length;
    ret = f_truncate(ff);
    if (ret != FR_OK) {
      ret = -EINVAL;
    }
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_readdir
 *
 * Description: Read the next directory entry
 *
 ****************************************************************************/

static int fat_readdir(FAR struct inode *mountpt,
                       FAR struct fs_dirent_s *dir,
                       FAR struct dirent *entry)
{
    FF_DIR*fdir;
    FAR struct fat_mountpt_s *fs;
    int ret;
    FILINFO file_info;

    /* Sanity checks */

    DEBUGASSERT(mountpt != NULL && mountpt->i_private != NULL);

    /* Recover our private data from the inode instance */

    fs = mountpt->i_private;
    fdir = (FF_DIR*)dir;

    /* Make sure that the mount is still healthy.
    * REVISIT: What if a forced unmount was done since opendir() was called?
    */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ret = f_readdir(fdir, &file_info);
    if (ret != FR_OK) {
        ret = -EINVAL;
        goto errout_with_lock;
    }

    strcpy(entry->d_name, file_info.fname);
    if ((file_info.fattrib & AM_DIR) == 0) {
        entry->d_type = DTYPE_FILE;
    } else {
        entry->d_type = DTYPE_DIRECTORY;
    }

    nxmutex_unlock(&fs->fs_lock);
    return OK;

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_rewindir
 *
 * Description: Reset directory read to the first entry
 *
 ****************************************************************************/

static int fat_rewinddir(FAR struct inode *mountpt,
                         FAR struct fs_dirent_s *dir)
{
    FF_DIR*fdir;
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(mountpt != NULL && mountpt->i_private != NULL);

    /* Recover our private data from the inode instance */

    fs = mountpt->i_private;
    fdir = (FF_DIR*)dir;

    /* Make sure that the mount is still healthy
    * REVISIT: What if a forced unmount was done since opendir() was called?
    */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }
    ret = f_readdir(fdir, NULL);
    if (ret != FR_OK) {
        ret = -EINVAL;
    }
    nxmutex_unlock(&fs->fs_lock);
    return ERROR;
}

/****************************************************************************
 * Name: fat_bind
 *
 * Description: This implements a portion of the mount operation. This
 *  function allocates and initializes the mountpoint private data and
 *  binds the blockdriver inode to the filesystem private data.  The final
 *  binding of the private data (containing the blockdriver) to the
 *  mountpoint is performed by mount().
 *
 ****************************************************************************/

static int fat_bind(FAR struct inode *blkdriver, FAR const void *data,
                    FAR void **handle)
{
    struct fat_mountpt_s *fs;
    int ret;

    /* Open the block driver */

    if (!blkdriver || !blkdriver->u.i_bops) {
        return -ENODEV;
    }

    if (data == NULL) {
        return -ENODEV;
    }

    if (blkdriver->u.i_bops->open &&
        blkdriver->u.i_bops->open(blkdriver) != OK) {
        return -ENODEV;
    }

    const TCHAR *rp = (const TCHAR *)data;
    BYTE volumeid = get_ldnumber(&rp);
    if (volumeid >= FATATTR_VOLUMEID) {
        return -ENODEV;
    }

    regist_inode[volumeid] = blkdriver;

    /* Create an instance of the mountpt state structure */

    fs = (struct fat_mountpt_s *)kmm_zalloc(sizeof(struct fat_mountpt_s));
    if (!fs) {
        return -ENOMEM;
    }
    fs->ff_fs = (FATFS *)kmm_zalloc(sizeof(FATFS));
    if (!fs->ff_fs) {
        kmm_free(fs);
        return -ENOMEM;
    }

    /* Initialize the allocated mountpt state structure.  The filesystem is
    * responsible for one reference on the blkdriver inode and does not
    * have to addref() here (but does have to release in unbind().
    */

    fs->fs_blkdriver = blkdriver;   /* Save the block driver reference */
    nxmutex_init(&fs->fs_lock);     /* Initialize the mutex that controls access */

    /* Then get information about the FAT32 filesystem on the devices managed
    * by this block driver.
    */

    ret = f_mount(fs->ff_fs, (const TCHAR *)data, 1);
    if (ret == FR_NO_FILESYSTEM) {
        MKFS_PARM opt = { 0 };
        opt.fmt = FM_ANY;
        char *work_buffer = kmm_zalloc(FF_MAX_SS * sizeof(char));
        if (work_buffer == NULL) {
            nxmutex_destroy(&fs->fs_lock);
            kmm_free(fs->ff_fs);
            kmm_free(fs);
            return -1;
        }
        ret = f_mkfs((const TCHAR *)data, &opt, work_buffer, FF_MAX_SS);
        kmm_free(work_buffer);
        if (ret != 0) {
            nxmutex_destroy(&fs->fs_lock);
            kmm_free(fs->ff_fs);
            kmm_free(fs);
            return -1;
        }
        ret = f_mount(NULL, (const TCHAR *)data, 1);
        ret += f_mount(fs->ff_fs, (const TCHAR *)data, 1);
        if (ret != 0) {
            nxmutex_destroy(&fs->fs_lock);
            kmm_free(fs->ff_fs);
            kmm_free(fs);
            return -1;
        }
    }

    memset(fs->name, 0, NAME_MAX + 1);
    memcpy(fs->name, data, NAME_MAX);

    *handle = (FAR void *)fs;
    return OK;
}

/****************************************************************************
 * Name: fat_unbind
 *
 * Description: This implements the filesystem portion of the umount
 *   operation.
 *
 ****************************************************************************/

static int fat_unbind(FAR void *handle, FAR struct inode **blkdriver,
                      unsigned int flags)
{
    FAR struct fat_mountpt_s *fs = (FAR struct fat_mountpt_s *)handle;
    int ret;

    if (!fs) {
        return -EINVAL;
    }

    /* Check if there are sill any files opened on the filesystem. */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    if (fs->ff_fs) {
        ret = f_unmount(fs->name);
        if (ret != FR_OK) {
            nxmutex_unlock(&fs->fs_lock);
            return -EINVAL;
        }
      }

    /* Unmount ... close the block driver */

    if (fs->fs_blkdriver) {
        FAR struct inode *inode = fs->fs_blkdriver;
        int i = 0;
        if (inode) {
            if (inode->u.i_bops && inode->u.i_bops->close) {
                inode->u.i_bops->close(inode);
            }

            /* We hold a reference to the block driver but should not but
            * mucking with inodes in this context.  So, we will just return
            * our contained reference to the block driver inode and let the
            * umount logic dispose of it.
            */

            if (blkdriver) {
                *blkdriver = inode;
            }
        }
        for (i = 0; i < FATATTR_VOLUMEID; i++) {
            if (regist_inode[i] == inode) {
                break;
            }
        }
        regist_inode[i] = NULL;
    }

    nxmutex_destroy(&fs->fs_lock);
    kmm_free(fs->ff_fs);
    kmm_free(fs);
    return OK;
}

/****************************************************************************
 * Name: fat_statfs
 *
 * Description: Return filesystem statistics
 *
 ****************************************************************************/

static int fat_statfs(FAR struct inode *mountpt, FAR struct statfs *buf)
{
    FAR struct fat_mountpt_s *fs;
    int ret;
    WORD fs_hwsectorsize = FF_MAX_SS;
    /* Sanity checks */

    DEBUGASSERT(mountpt && mountpt->i_private);

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
      return ret;
    }

    /* Fill in the statfs info */
    buf->f_type    = MSDOS_SUPER_MAGIC;

    /* We will claim that the optimal transfer size is the size of a cluster
    * in bytes.
    */
#if FF_MAX_SS != FF_MIN_SS
    fs_hwsectorsize = fs->ff_fs->ssize;
#endif
    buf->f_bsize   = fs->ff_fs->csize * fs_hwsectorsize;
#if !FF_FS_READONLY
    buf->f_blocks = fs->ff_fs->last_clst;
    buf->f_bavail = fs->ff_fs->free_clst;
#endif
    /* Everything else follows in units of clusters */

#if FF_USE_LFN
    buf->f_namelen = FF_LFN_BUF;         /* Maximum length of filenames */
#else
    buf->f_namelen = (12 + 1);           /* Maximum length of filenames */
#endif
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_unlink
 *
 * Description: Remove a file
 *
 ****************************************************************************/

static int fat_unlink(FAR struct inode *mountpt, FAR const char *relpath)
{
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(mountpt && mountpt->i_private);

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ret = f_unlink(relpath);
    if (ret != FR_OK) {
        ret = -EINVAL;
    }

    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_mkdir
 *
 * Description: Create a directory
 *
 ****************************************************************************/

static int fat_mkdir(FAR struct inode *mountpt, FAR const char *relpath,
                     mode_t mode)
{
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(mountpt && mountpt->i_private);

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ret = f_mkdir(relpath);
    if (ret != FR_OK) {
        ret = -EINVAL;
    }
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_rmdir
 *
 * Description: Remove a directory
 *
 ****************************************************************************/

static int fat_rmdir(FAR struct inode *mountpt, FAR const char *relpath)
{
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(mountpt && mountpt->i_private);

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ret = f_rmdir(relpath);
    if (ret != FR_OK) {
        ret = -EINVAL;
    }

    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_rename
 *
 * Description: Rename a file or directory
 *
 ****************************************************************************/

static int fat_rename(FAR struct inode *mountpt, FAR const char *oldrelpath,
                      FAR const char *newrelpath)
{
    FAR struct fat_mountpt_s *fs;
    int ret;

    /* Sanity checks */

    DEBUGASSERT(mountpt && mountpt->i_private);

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ret = f_rename(oldrelpath, newrelpath);
    if (ret != FR_OK) {
        ret = -EINVAL;
    }
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Name: fat_fattime2systime
 *
 * Description:
 *   Convert FAT data and time to a system time_t
 *
 *    16-bit FAT time:
 *      Bits 0:4   = 2 second count (0-29 representing 0-58 seconds)
 *      Bits 5-10  = minutes (0-59)
 *      Bits 11-15 = hours (0-23)
 *    16-bit FAT date:
 *      Bits 0:4   = Day of month (1-31)
 *      Bits 5:8   = Month of year (1-12)
 *      Bits 9:15  = Year from 1980 (0-127 representing 1980-2107)
 *
 ****************************************************************************/

time_t fat_fattime2systime(uint16_t fattime, uint16_t fatdate)
{
  /* Unless you have a hardware RTC or some other to get accurate time, then
   * there is no reason to support FAT time.
   */
  struct tm tm;
  unsigned int tmp;

  /* Break out the date and time */

  tm.tm_sec  =  (fattime & 0x001f) <<  1;       /* Bits 0-4: 2 second count (0-29) */
  tm.tm_min  =  (fattime & 0x07e0) >>  5;       /* Bits 5-10: minutes (0-59) */
  tm.tm_hour =  (fattime & 0xf800) >> 11;       /* Bits 11-15: hours (0-23) */

  tm.tm_mday =  (fatdate & 0x001f);             /* Bits 0-4: Day of month (1-31) */
  tmp        = ((fatdate & 0x01e0) >>  5);      /* Bits 5-8: Month of year (1-12) */
  tm.tm_mon  =   tmp > 0 ? tmp - 1 : 0;
  tm.tm_year = ((fatdate & 0xfe00) >>  9) + 80; /* Bits 9-15: Year from 1980 */

  /* Then convert the broken out time into seconds since the epoch */

  return timegm(&tm);
}

/****************************************************************************
 * Name: fat_stat_common
 *
 * Description:
 *   Common logic used by fat_stat_file() and fat_fstat_root().
 *
 ****************************************************************************/

static int fat_stat_common(FAR struct fat_mountpt_s *fs,
                           FILINFO *file_info, FAR struct stat *buf)
{
    uint16_t fatdate;
    uint16_t fattime;
    WORD fs_hwsectorsize = FF_MAX_SS;

    /* Times */

    fatdate           = file_info->fdate;
    fattime           = file_info->ftime;
    buf->st_mtime     = fat_fattime2systime(fattime, fatdate);
    buf->st_atime     = buf->st_mtime;
    buf->st_ctime     = fat_fattime2systime(fattime, fatdate);

    /* File/directory size, access block size */

    buf->st_size      = file_info->fsize;

#if FF_MAX_SS != FF_MIN_SS
    fs_hwsectorsize = fs->ff_fs->ssize;
#endif
    buf->st_blksize   = fs->ff_fs->csize * fs_hwsectorsize;
    buf->st_blocks    = (buf->st_size + buf->st_blksize - 1) / buf->st_blksize;

    return OK;
}

/****************************************************************************
 * Name: fat_stat_file
 *
 * Description:
 *   Function to return the status associated with a file in the FAT file
 *   system.  Used by fat_stat() and fat_fstat().
 *
 ****************************************************************************/

static int fat_stat_file(FAR struct fat_mountpt_s *fs,
                         FILINFO *file_info, FAR struct stat *buf)
{
    BYTE attribute;

    /* Initialize the "struct stat" */

    memset(buf, 0, sizeof(struct stat));

    /* Get attribute from direntry */

    attribute = file_info->fattrib;
    if ((attribute & FATATTR_VOLUMEID) != 0) {
        return -ENOENT;
    }

    /* Set the access permissions.  The file/directory is always readable
    * by everyone but may be writeable by no-one.
    */

    buf->st_mode = S_IROTH | S_IRGRP | S_IRUSR;
    if ((attribute & AM_RDO) == 0) {
        buf->st_mode |= S_IWOTH | S_IWGRP | S_IWUSR;
    }

    /* We will report only types file or directory */

    if ((attribute & AM_DIR) != 0) {
        buf->st_mode |= S_IFDIR;
    } else {
        buf->st_mode |= S_IFREG;
    }

    return fat_stat_common(fs, file_info, buf);
}

/****************************************************************************
 * Name: fat_stat
 *
 * Description: Return information about a file or directory
 *
 ****************************************************************************/

static int fat_stat(FAR struct inode *mountpt, FAR const char *relpath,
                    FAR struct stat *buf)
{
    FAR struct fat_mountpt_s *fs;
    int ret;
    FILINFO fno;

    /* Sanity checks */

    DEBUGASSERT(mountpt && mountpt->i_private);

    /* Get the mountpoint private data from the inode structure */

    fs = mountpt->i_private;

    /* Check if the mount is still healthy */

    ret = nxmutex_lock(&fs->fs_lock);
    if (ret < 0) {
        return ret;
    }

    ret = f_stat(relpath, &fno);
    if (ret != FR_OK) {
        ret = -EINVAL;
        goto errout_with_lock;
    }
    
    ret = fat_stat_file(fs, &fno, buf);

errout_with_lock:
    nxmutex_unlock(&fs->fs_lock);
    return ret;
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/
