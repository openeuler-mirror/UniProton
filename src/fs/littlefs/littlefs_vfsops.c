/****************************************************************************
 * fs/littlefs/littlefs_vfsops.c
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

#include "lfs.h"

#define LITTLEFS_SUPER_MAGIC 0x20170118
#define LITTLEFS_DEFAULT_CACHE_SIZE 256
#define LITTLEFS_DEFAULT_LOOKAHEAD_SIZE 32

struct up_littlefs_s {
    lfs_t lfs;
    struct lfs_config cfg;
    struct inode *mtd_inode;
    struct mtd_dev_s *mtd;
    struct mtd_geometry_s geo;
    mutex_t lock;
};

static void littlefs_name_copy(char *dst, size_t dst_size, const char *src)
{
    if (dst_size == 0) {
        return;
    }

    (void)strncpy(dst, src, dst_size - 1);
    dst[dst_size - 1] = '\0';
}

static int littlefs_errno(int ret)
{
    return ret < 0 ? ret : OK;
}

static const char *littlefs_path(const char *relpath)
{
    while (relpath != NULL && *relpath == '/') {
        relpath++;
    }

    return relpath == NULL || *relpath == '\0' ? "." : relpath;
}

static int littlefs_read(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, void *dst, lfs_size_t size)
{
    struct up_littlefs_s *priv = cfg->context;
    off_t addr = (off_t)block * cfg->block_size + off;

    return MTD_READ(priv->mtd, addr, size, dst) == (ssize_t)size ? 0 : LFS_ERR_IO;
}

static int littlefs_prog(const struct lfs_config *cfg, lfs_block_t block, lfs_off_t off, const void *src, lfs_size_t size)
{
    struct up_littlefs_s *priv = cfg->context;
    off_t addr = (off_t)block * cfg->block_size + off;

    return MTD_WRITE(priv->mtd, addr, size, src) == (ssize_t)size ? 0 : LFS_ERR_IO;
}

static int littlefs_erase(const struct lfs_config *cfg, lfs_block_t block)
{
    struct up_littlefs_s *priv = cfg->context;

    return MTD_ERASE(priv->mtd, block, 1) == 1 ? 0 : LFS_ERR_IO;
}

static int littlefs_sync(const struct lfs_config *cfg)
{
    (void)cfg;
    return 0;
}

static int littlefs_flags(int oflags)
{
    int flags = 0;

    if ((oflags & O_CREAT) != 0) {
        flags |= LFS_O_CREAT;
    }
    if ((oflags & O_EXCL) != 0) {
        flags |= LFS_O_EXCL;
    }
    if ((oflags & O_TRUNC) != 0) {
        flags |= LFS_O_TRUNC;
    }
    if ((oflags & O_APPEND) != 0) {
        flags |= LFS_O_APPEND;
    }

    if ((oflags & O_ACCMODE) == O_RDONLY) {
        flags |= LFS_O_RDONLY;
    } else if ((oflags & O_ACCMODE) == O_WRONLY) {
        flags |= LFS_O_WRONLY;
    } else {
        flags |= LFS_O_RDWR;
    }

    return flags;
}

static int littlefs_open(struct file *filep, const char *relpath, int oflags, mode_t mode)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    lfs_file_t *file;
    int ret;

    (void)mode;
    file = kmm_zalloc(sizeof(*file));
    if (file == NULL) {
        return -ENOMEM;
    }

    nxmutex_lock(&priv->lock);
    ret = lfs_file_open(&priv->lfs, file, littlefs_path(relpath), littlefs_flags(oflags));
    nxmutex_unlock(&priv->lock);
    if (ret < 0) {
        kmm_free(file);
        return littlefs_errno(ret);
    }

    filep->f_priv = file;
    return OK;
}

static int littlefs_close(struct file *filep)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    lfs_file_t *file = filep->f_priv;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_file_close(&priv->lfs, file);
    nxmutex_unlock(&priv->lock);
    kmm_free(file);
    return littlefs_errno(ret);
}

static ssize_t littlefs_read_file(struct file *filep, char *buffer, size_t buflen)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_file_read(&priv->lfs, filep->f_priv, buffer, buflen);
    nxmutex_unlock(&priv->lock);
    return ret < 0 ? littlefs_errno(ret) : ret;
}

static ssize_t littlefs_write_file(struct file *filep, const char *buffer, size_t buflen)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_file_write(&priv->lfs, filep->f_priv, buffer, buflen);
    nxmutex_unlock(&priv->lock);
    return ret < 0 ? littlefs_errno(ret) : ret;
}

static off_t littlefs_seek(struct file *filep, off_t offset, int whence)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_file_seek(&priv->lfs, filep->f_priv, offset, whence);
    nxmutex_unlock(&priv->lock);
    return ret < 0 ? littlefs_errno(ret) : ret;
}

static int littlefs_truncate(struct file *filep, off_t length)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_file_truncate(&priv->lfs, filep->f_priv, length);
    nxmutex_unlock(&priv->lock);
    return littlefs_errno(ret);
}

static int littlefs_sync_file(struct file *filep)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_file_sync(&priv->lfs, filep->f_priv);
    nxmutex_unlock(&priv->lock);
    return littlefs_errno(ret);
}

static int littlefs_fstat(const struct file *filep, struct stat *buf)
{
    struct up_littlefs_s *priv = filep->f_inode->i_private;
    lfs_soff_t size;

    memset(buf, 0, sizeof(*buf));
    nxmutex_lock(&priv->lock);
    size = lfs_file_size(&priv->lfs, filep->f_priv);
    nxmutex_unlock(&priv->lock);
    if (size < 0) {
        return littlefs_errno(size);
    }
    buf->st_mode = S_IFREG | 0666;
    buf->st_size = size;
    return OK;
}

static int littlefs_opendir(struct inode *mountpt, const char *relpath, struct fs_dirent_s **dir)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    lfs_dir_t *ldir;
    int ret;

    ldir = kmm_zalloc(sizeof(*ldir));
    if (ldir == NULL) {
        return -ENOMEM;
    }

    nxmutex_lock(&priv->lock);
    ret = lfs_dir_open(&priv->lfs, ldir, littlefs_path(relpath));
    nxmutex_unlock(&priv->lock);
    if (ret < 0) {
        kmm_free(ldir);
        return littlefs_errno(ret);
    }

    *dir = (struct fs_dirent_s *)ldir;
    return OK;
}

static int littlefs_closedir(struct inode *mountpt, struct fs_dirent_s *dir)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_dir_close(&priv->lfs, (lfs_dir_t *)dir);
    nxmutex_unlock(&priv->lock);
    kmm_free(dir);
    return littlefs_errno(ret);
}

static int littlefs_readdir(struct inode *mountpt, struct fs_dirent_s *dir, struct dirent *entry)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    struct lfs_info info;
    int ret;

    nxmutex_lock(&priv->lock);
    do {
        ret = lfs_dir_read(&priv->lfs, (lfs_dir_t *)dir, &info);
    } while (ret > 0 && (strcmp(info.name, ".") == 0 || strcmp(info.name, "..") == 0));
    nxmutex_unlock(&priv->lock);

    if (ret < 0) {
        return littlefs_errno(ret);
    }
    if (ret == 0) {
        return -ENOENT;
    }

    littlefs_name_copy(entry->d_name, sizeof(entry->d_name), info.name);
    entry->d_type = info.type == LFS_TYPE_DIR ? DTYPE_DIRECTORY : DTYPE_FILE;
    return OK;
}

static int littlefs_bind(struct inode *mtd_inode, const void *data, void **handle)
{
    struct up_littlefs_s *priv;
    int ret;

    (void)data;
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

    priv->cfg.context = priv;
    priv->cfg.read = littlefs_read;
    priv->cfg.prog = littlefs_prog;
    priv->cfg.erase = littlefs_erase;
    priv->cfg.sync = littlefs_sync;
    priv->cfg.read_size = priv->geo.blocksize;
    priv->cfg.prog_size = priv->geo.blocksize;
    priv->cfg.block_size = priv->geo.erasesize;
    priv->cfg.block_count = priv->geo.neraseblocks;
    priv->cfg.cache_size = priv->geo.blocksize > LITTLEFS_DEFAULT_CACHE_SIZE ? priv->geo.blocksize : LITTLEFS_DEFAULT_CACHE_SIZE;
    priv->cfg.lookahead_size = LITTLEFS_DEFAULT_LOOKAHEAD_SIZE;
    priv->cfg.block_cycles = 500;

    nxmutex_init(&priv->lock);
    ret = lfs_mount(&priv->lfs, &priv->cfg);
    if (ret < 0) {
        ret = lfs_format(&priv->lfs, &priv->cfg);
        if (ret == 0) {
            ret = lfs_mount(&priv->lfs, &priv->cfg);
        }
    }
    if (ret < 0) {
        nxmutex_destroy(&priv->lock);
        kmm_free(priv);
        return littlefs_errno(ret);
    }

    *handle = priv;
    return OK;
}

static int littlefs_unbind(void *handle, struct inode **driver, unsigned int flags)
{
    struct up_littlefs_s *priv = handle;

    (void)flags;
    if (priv == NULL) {
        return -EINVAL;
    }

    nxmutex_lock(&priv->lock);
    (void)lfs_unmount(&priv->lfs);
    nxmutex_unlock(&priv->lock);
    nxmutex_destroy(&priv->lock);
    if (driver != NULL) {
        *driver = priv->mtd_inode;
    }
    kmm_free(priv);
    return OK;
}

static int littlefs_statfs(struct inode *mountpt, struct statfs *buf)
{
    struct up_littlefs_s *priv = mountpt->i_private;

    memset(buf, 0, sizeof(*buf));
    buf->f_type = LITTLEFS_SUPER_MAGIC;
    buf->f_bsize = priv->cfg.block_size;
    buf->f_blocks = priv->cfg.block_count;
    buf->f_namelen = LFS_NAME_MAX;
    return OK;
}

static int littlefs_unlink(struct inode *mountpt, const char *relpath)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_remove(&priv->lfs, littlefs_path(relpath));
    nxmutex_unlock(&priv->lock);
    return littlefs_errno(ret);
}

static int littlefs_mkdir(struct inode *mountpt, const char *relpath, mode_t mode)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    int ret;

    (void)mode;
    nxmutex_lock(&priv->lock);
    ret = lfs_mkdir(&priv->lfs, littlefs_path(relpath));
    nxmutex_unlock(&priv->lock);
    return littlefs_errno(ret);
}

static int littlefs_rmdir(struct inode *mountpt, const char *relpath)
{
    return littlefs_unlink(mountpt, relpath);
}

static int littlefs_rename(struct inode *mountpt, const char *oldrelpath, const char *newrelpath)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    int ret;

    nxmutex_lock(&priv->lock);
    ret = lfs_rename(&priv->lfs, littlefs_path(oldrelpath), littlefs_path(newrelpath));
    nxmutex_unlock(&priv->lock);
    return littlefs_errno(ret);
}

static int littlefs_stat(struct inode *mountpt, const char *relpath, struct stat *buf)
{
    struct up_littlefs_s *priv = mountpt->i_private;
    struct lfs_info info;
    int ret;

    memset(buf, 0, sizeof(*buf));
    nxmutex_lock(&priv->lock);
    ret = lfs_stat(&priv->lfs, littlefs_path(relpath), &info);
    nxmutex_unlock(&priv->lock);
    if (ret < 0) {
        return littlefs_errno(ret);
    }
    buf->st_mode = info.type == LFS_TYPE_DIR ? (S_IFDIR | 0777) : (S_IFREG | 0666);
    buf->st_size = info.size;
    return OK;
}

const struct mountpt_operations g_littlefs_operations = {
    littlefs_open, littlefs_close, littlefs_read_file, littlefs_write_file,
    littlefs_seek, NULL, NULL, littlefs_truncate, littlefs_sync_file, NULL,
    littlefs_fstat, NULL, littlefs_opendir, littlefs_closedir,
    littlefs_readdir, NULL, littlefs_bind, littlefs_unbind, littlefs_statfs,
    littlefs_unlink, littlefs_mkdir, littlefs_rmdir, littlefs_rename,
    littlefs_stat, NULL
};
