/****************************************************************************
 * fs/spiffs/spiffs_vfsops.c
 ****************************************************************************/

#include <nuttx/config.h>

#include <sys/stat.h>
#include <sys/statfs.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <nuttx/fs/fs.h>
#include <nuttx/kmalloc.h>
#include <nuttx/mtd/mtd.h>

#include "inode/inode.h"
#include "spiffs.h"
#include "spiffs_nucleus.h"

#define SPIFFS_SUPER_MAGIC 0x73706966
#define SPIFFS_DEFAULT_BLOCK_SIZE 4096
#define SPIFFS_DEFAULT_PAGE_SIZE 256
#define SPIFFS_FD_COUNT 8
#define SPIFFS_CACHE_PAGES 4

struct up_spiffs_s {
    spiffs fs;
    struct inode *mtd_inode;
    struct mtd_dev_s *mtd;
    struct mtd_geometry_s geo;
    uint8_t *work;
    uint8_t *fds;
    uint8_t *cache;
    uint32_t fd_size;
    uint32_t cache_size;
    mutex_t lock;
};

static void spiffs_name_copy(char *dst, size_t dst_size, const char *src)
{
    if (dst_size == 0) {
        return;
    }

    (void)strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static int spiffs_errno(int ret)
{
    switch (ret) {
        case SPIFFS_OK:
            return OK;
        case SPIFFS_ERR_NOT_FOUND:
        case SPIFFS_ERR_NOT_A_FILE:
        case SPIFFS_ERR_DELETED:
        case SPIFFS_ERR_FILE_DELETED:
            return -ENOENT;
        case SPIFFS_ERR_FILE_EXISTS:
        case SPIFFS_ERR_MOUNTED:
            return -EEXIST;
        case SPIFFS_ERR_FULL:
        case SPIFFS_ERR_PROBE_TOO_FEW_BLOCKS:
            return -ENOSPC;
        case SPIFFS_ERR_BAD_DESCRIPTOR:
        case SPIFFS_ERR_OUT_OF_FILE_DESCS:
            return -EBADF;
        case SPIFFS_ERR_NAME_TOO_LONG:
            return -ENAMETOOLONG;
        case SPIFFS_ERR_NOT_WRITABLE:
        case SPIFFS_ERR_NOT_READABLE:
        case SPIFFS_ERR_NOT_CONFIGURED:
            return -EACCES;
        case SPIFFS_ERR_NOT_A_FS:
        case SPIFFS_ERR_PROBE_NOT_A_FS:
        case SPIFFS_ERR_MAGIC_NOT_POSSIBLE:
            return -ENODEV;
        default:
            return -EIO;
    }
}

static int up_spiffs_flags(int oflags)
{
    int flags = 0;

    if ((oflags & O_ACCMODE) == O_RDONLY) {
        flags |= SPIFFS_O_RDONLY;
    } else if ((oflags & O_ACCMODE) == O_WRONLY) {
        flags |= SPIFFS_O_WRONLY;
    } else {
        flags |= SPIFFS_O_RDWR;
    }

    if ((oflags & O_CREAT) != 0) {
        flags |= SPIFFS_O_CREAT;
    }
    if ((oflags & O_EXCL) != 0) {
        flags |= SPIFFS_O_EXCL;
    }
    if ((oflags & O_TRUNC) != 0) {
        flags |= SPIFFS_O_TRUNC;
    }
    if ((oflags & O_APPEND) != 0) {
        flags |= SPIFFS_O_APPEND;
    }

    return flags;
}

static struct up_spiffs_s *spiffs_priv(spiffs *fs)
{
    return (struct up_spiffs_s *)fs->user_data;
}

static s32_t spiffs_mtd_read(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *dst)
{
    struct up_spiffs_s *priv = spiffs_priv(fs);

    if (MTD_READ(priv->mtd, addr, size, dst) == (ssize_t)size) {
        return SPIFFS_OK;
    }
    return SPIFFS_ERR_INTERNAL;
}

static s32_t spiffs_mtd_write(struct spiffs_t *fs, u32_t addr, u32_t size, u8_t *src)
{
    struct up_spiffs_s *priv = spiffs_priv(fs);

    if (MTD_WRITE(priv->mtd, addr, size, src) == (ssize_t)size) {
        return SPIFFS_OK;
    }
    return SPIFFS_ERR_INTERNAL;
}

static s32_t spiffs_mtd_erase(struct spiffs_t *fs, u32_t addr, u32_t size)
{
    struct up_spiffs_s *priv = spiffs_priv(fs);
    off_t block = addr / priv->geo.erasesize;
    size_t count = size / priv->geo.erasesize;

    return MTD_ERASE(priv->mtd, block, count) == (ssize_t)count ? SPIFFS_OK : SPIFFS_ERR_ERASE_FAIL;
}

static int up_spiffs_open(struct file *filep, const char *relpath, int oflags, mode_t mode)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    spiffs_file fd;
    int ret;

    (void)mode;
    ret = nxmutex_lock(&priv->lock);
    if (ret < 0) {
        return ret;
    }
    fd = SPIFFS_open(&priv->fs, relpath, up_spiffs_flags(oflags), 0);
    nxmutex_unlock(&priv->lock);
    if (fd < SPIFFS_OK) {
        return spiffs_errno(fd);
    }

    filep->f_priv = (void *)(uintptr_t)fd;
    return OK;
}

static int up_spiffs_close(struct file *filep)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_close(&priv->fs, (spiffs_file)(uintptr_t)filep->f_priv);
    nxmutex_unlock(&priv->lock);
    return spiffs_errno(ret);
}

static ssize_t up_spiffs_read(struct file *filep, char *buffer, size_t buflen)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    s32_t ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_read(&priv->fs, (spiffs_file)(uintptr_t)filep->f_priv, buffer, buflen);
    nxmutex_unlock(&priv->lock);
    return ret < 0 ? spiffs_errno(ret) : ret;
}

static ssize_t up_spiffs_write(struct file *filep, const char *buffer, size_t buflen)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    s32_t ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_write(&priv->fs, (spiffs_file)(uintptr_t)filep->f_priv, (void *)buffer, buflen);
    nxmutex_unlock(&priv->lock);
    return ret < 0 ? spiffs_errno(ret) : ret;
}

static off_t up_spiffs_seek(struct file *filep, off_t offset, int whence)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    s32_t ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_lseek(&priv->fs, (spiffs_file)(uintptr_t)filep->f_priv, offset, whence);
    nxmutex_unlock(&priv->lock);
    return ret < 0 ? spiffs_errno(ret) : ret;
}

static int up_spiffs_sync(struct file *filep)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_fflush(&priv->fs, (spiffs_file)(uintptr_t)filep->f_priv);
    nxmutex_unlock(&priv->lock);
    return spiffs_errno(ret);
}

static int up_spiffs_fstat(const struct file *filep, struct stat *buf)
{
    struct up_spiffs_s *priv = filep->f_inode->i_private;
    spiffs_stat s;
    int ret;

    memset(buf, 0, sizeof(*buf));
    nxmutex_lock(&priv->lock);
    ret = SPIFFS_fstat(&priv->fs, (spiffs_file)(uintptr_t)filep->f_priv, &s);
    nxmutex_unlock(&priv->lock);
    if (ret < 0) {
        return spiffs_errno(ret);
    }
    buf->st_mode = s.type == SPIFFS_TYPE_DIR ? (S_IFDIR | 0777) : (S_IFREG | 0666);
    buf->st_size = s.size;
    return OK;
}

static int up_spiffs_opendir(struct inode *mountpt, const char *relpath, struct fs_dirent_s **dir)
{
    struct up_spiffs_s *priv = mountpt->i_private;
    spiffs_DIR *sdir;

    sdir = kmm_zalloc(sizeof(*sdir));
    if (sdir == NULL) {
        return -ENOMEM;
    }

    nxmutex_lock(&priv->lock);
    if (SPIFFS_opendir(&priv->fs, relpath, sdir) == NULL) {
        nxmutex_unlock(&priv->lock);
        kmm_free(sdir);
        return -ENOENT;
    }
    nxmutex_unlock(&priv->lock);
    *dir = (struct fs_dirent_s *)sdir;
    return OK;
}

static int up_spiffs_closedir(struct inode *mountpt, struct fs_dirent_s *dir)
{
    struct up_spiffs_s *priv = mountpt->i_private;
    spiffs_DIR *sdir = (spiffs_DIR *)dir;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_closedir(sdir);
    nxmutex_unlock(&priv->lock);
    kmm_free(sdir);
    return spiffs_errno(ret);
}

static int up_spiffs_readdir(struct inode *mountpt, struct fs_dirent_s *dir, struct dirent *entry)
{
    struct spiffs_dirent dent;
    struct up_spiffs_s *priv = mountpt->i_private;

    nxmutex_lock(&priv->lock);
    if (SPIFFS_readdir((spiffs_DIR *)dir, &dent) == NULL) {
        nxmutex_unlock(&priv->lock);
        return -ENOENT;
    }
    nxmutex_unlock(&priv->lock);

    spiffs_name_copy(entry->d_name, sizeof(entry->d_name), (const char *)dent.name);
    entry->d_type = dent.type == SPIFFS_TYPE_DIR ? DTYPE_DIRECTORY : DTYPE_FILE;
    return OK;
}

static int up_spiffs_bind(struct inode *mtd_inode, const void *data, void **handle)
{
    struct up_spiffs_s *priv;
    spiffs_config cfg;
    int ret;
    uint32_t page_size;

    if (mtd_inode == NULL || mtd_inode->u.i_mtd == NULL) {
        return -ENODEV;
    }

    priv = kmm_zalloc(sizeof(*priv));
    if (priv == NULL) {
        return -ENOMEM;
    }

    priv->mtd_inode = mtd_inode;
    priv->mtd = mtd_inode->u.i_mtd;
    ret = MTD_IOCTL(priv->mtd, MTDIOC_GEOMETRY, (unsigned long)&priv->geo);
    if (ret < 0) {
        kmm_free(priv);
        return ret;
    }

    page_size = priv->geo.blocksize == 0 ? SPIFFS_DEFAULT_PAGE_SIZE : priv->geo.blocksize;
    if (page_size > SPIFFS_DEFAULT_PAGE_SIZE) {
        page_size = SPIFFS_DEFAULT_PAGE_SIZE;
    }

    priv->fd_size = sizeof(spiffs_fd) * SPIFFS_FD_COUNT;
    priv->cache_size = ((page_size + 32) * SPIFFS_CACHE_PAGES) + 40;
    priv->work = kmm_zalloc(page_size * 2);
    priv->fds = kmm_zalloc(priv->fd_size);
    priv->cache = kmm_zalloc(priv->cache_size);
    if (priv->work == NULL || priv->fds == NULL || priv->cache == NULL) {
        ret = -ENOMEM;
        goto err_free;
    }

    memset(&cfg, 0, sizeof(cfg));
    cfg.hal_read_f = spiffs_mtd_read;
    cfg.hal_write_f = spiffs_mtd_write;
    cfg.hal_erase_f = spiffs_mtd_erase;
    cfg.phys_size = priv->geo.erasesize * priv->geo.neraseblocks;
    cfg.phys_addr = 0;
    cfg.phys_erase_block = priv->geo.erasesize;
    cfg.log_block_size = priv->geo.erasesize == 0 ? SPIFFS_DEFAULT_BLOCK_SIZE : priv->geo.erasesize;
    cfg.log_page_size = page_size;
    cfg.fh_ix_offset = TEST_SPIFFS_FILEHDL_OFFSET;

    nxmutex_init(&priv->lock);
    priv->fs.user_data = priv;
    ret = SPIFFS_mount(&priv->fs, &cfg, priv->work, priv->fds, priv->fd_size, priv->cache, priv->cache_size, NULL);
    if (ret == SPIFFS_ERR_NOT_A_FS) {
        (void)SPIFFS_format(&priv->fs);
        ret = SPIFFS_mount(&priv->fs, &cfg, priv->work, priv->fds, priv->fd_size, priv->cache, priv->cache_size, NULL);
    }
    if (ret != SPIFFS_OK) {
        nxmutex_destroy(&priv->lock);
        ret = spiffs_errno(ret);
        goto err_free;
    }

    *handle = priv;
    (void)data;
    return OK;

err_free:
    if (priv->work != NULL) {
        kmm_free(priv->work);
    }
    if (priv->fds != NULL) {
        kmm_free(priv->fds);
    }
    if (priv->cache != NULL) {
        kmm_free(priv->cache);
    }
    kmm_free(priv);
    return ret;
}

static int up_spiffs_unbind(void *handle, struct inode **driver, unsigned int flags)
{
    struct up_spiffs_s *priv = handle;

    (void)flags;
    if (priv == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&priv->lock);
    (void)SPIFFS_unmount(&priv->fs);
    nxmutex_unlock(&priv->lock);
    nxmutex_destroy(&priv->lock);
    if (driver != NULL) {
        *driver = priv->mtd_inode;
    }
    kmm_free(priv->work);
    kmm_free(priv->fds);
    kmm_free(priv->cache);
    kmm_free(priv);
    return OK;
}

static int up_spiffs_statfs(struct inode *mountpt, struct statfs *buf)
{
    struct up_spiffs_s *priv = mountpt->i_private;

    memset(buf, 0, sizeof(*buf));
    buf->f_type = SPIFFS_SUPER_MAGIC;
    buf->f_bsize = priv->geo.blocksize;
    buf->f_blocks = priv->geo.neraseblocks * (priv->geo.erasesize / priv->geo.blocksize);
    buf->f_namelen = SPIFFS_OBJ_NAME_LEN - 1;
    return OK;
}

static int up_spiffs_unlink(struct inode *mountpt, const char *relpath)
{
    struct up_spiffs_s *priv = mountpt->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_remove(&priv->fs, relpath);
    nxmutex_unlock(&priv->lock);
    return spiffs_errno(ret);
}

static int up_spiffs_rename(struct inode *mountpt, const char *oldrelpath, const char *newrelpath)
{
    struct up_spiffs_s *priv = mountpt->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = SPIFFS_rename(&priv->fs, oldrelpath, newrelpath);
    nxmutex_unlock(&priv->lock);
    return spiffs_errno(ret);
}

static int up_spiffs_stat(struct inode *mountpt, const char *relpath, struct stat *buf)
{
    struct up_spiffs_s *priv = mountpt->i_private;
    spiffs_stat s;
    int ret;

    memset(buf, 0, sizeof(*buf));
    nxmutex_lock(&priv->lock);
    ret = SPIFFS_stat(&priv->fs, relpath, &s);
    nxmutex_unlock(&priv->lock);
    if (ret < 0) {
        return spiffs_errno(ret);
    }
    buf->st_mode = s.type == SPIFFS_TYPE_DIR ? (S_IFDIR | 0777) : (S_IFREG | 0666);
    buf->st_size = s.size;
    return OK;
}

const struct mountpt_operations g_spiffs_operations = {
    up_spiffs_open, up_spiffs_close, up_spiffs_read, up_spiffs_write, up_spiffs_seek, NULL, NULL,
    NULL, up_spiffs_sync, NULL, up_spiffs_fstat, NULL, up_spiffs_opendir,
    up_spiffs_closedir, up_spiffs_readdir, NULL, up_spiffs_bind, up_spiffs_unbind,
    up_spiffs_statfs, up_spiffs_unlink, NULL, NULL, up_spiffs_rename, up_spiffs_stat, NULL
};
