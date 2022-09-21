/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-09-21
 * Description: littlefs适配层代码
 */
#define _GNU_SOURCE 1
#include "lfs_adapter.h"
#include "prt_fs.h"
#include "vfs_files.h"
#include "vfs_operations.h"
#include "vfs_partition.h"
#include "vfs_maps.h"
#include "vfs_mount.h"
#include "securec.h"

static pthread_mutex_t g_fsLocalMutex = PTHREAD_MUTEX_INITIALIZER;

static struct PartitionCfg g_partitionCfg;
static struct TagDeviceDesc *g_lfsDevice;

static uintptr_t OsLfsGetStartAddr(S32 partition)
{
    if (g_lfsDevice == NULL) {
        struct TagDeviceDesc *device = NULL;
        for (device = OsGetDeviceList(); device != NULL; device = device->dNext) {
            if (strcmp(device->dFsType, "littlefs") == 0) {
                g_lfsDevice = device;
                break;
            }
        }
    }

    if ((g_lfsDevice == NULL) || (partition >= g_lfsDevice->dPartNum)) {
        return INVALID_DEVICE_ADDR;
    }

    return g_lfsDevice->dAddrArray[partition];
}

static S32 OsLfsBlockRead(const struct lfs_config *c, lfs_block_t block,
                                      lfs_off_t off, void *dst, lfs_size_t size)
{
    uintptr_t startAddr = OsLfsGetStartAddr((S32)c->context);
    if (startAddr == INVALID_DEVICE_ADDR) {
        return -1;
    }
    startAddr += (c->block_size * block + off);
    return (g_partitionCfg.readFunc)((S32)c->context, startAddr, dst, size);
}

static S32 OsLfsBlockWrite(const struct lfs_config *c, lfs_block_t block,
                                       lfs_off_t off, const void *dst, lfs_size_t size)
{
    uintptr_t startAddr = OsLfsGetStartAddr((S32)c->context);
    if (startAddr == INVALID_DEVICE_ADDR) {
        return -1;
    }

    startAddr += (c->block_size * block + off);
    return (g_partitionCfg.writeFunc)((S32)c->context, startAddr, dst, size);
}

static S32 OsLfsBlockErase(const struct lfs_config *c, lfs_block_t block)
{
    uintptr_t startAddr = OsLfsGetStartAddr((S32)c->context);
    if (startAddr == INVALID_DEVICE_ADDR) {
        return -1;
    }

    startAddr += (c->block_size * block);
    return (g_partitionCfg.eraseFunc)((S32)c->context, startAddr, c->block_size);
}

static S32 OsLfsBlockSync(const struct lfs_config *c)
{
    (void)c;
    return 0;
}

static S32 OsConvertFlagToLfsOpenFlag(S32 oflags)
{
    S32 lfsOpenFlag = 0;

    if (oflags & O_CREAT) {
        lfsOpenFlag |= LFS_O_CREAT;
    }

    if (oflags & O_EXCL) {
        lfsOpenFlag |= LFS_O_EXCL;
    }

    if (oflags & O_TRUNC) {
        lfsOpenFlag |= LFS_O_TRUNC;
    }

    if (oflags & O_APPEND) {
        lfsOpenFlag |= LFS_O_APPEND;
    }

    if (oflags & O_RDWR) {
        lfsOpenFlag |= LFS_O_RDWR;
    }

    if (oflags & O_WRONLY) {
        lfsOpenFlag |= LFS_O_WRONLY;
    }

    if (oflags == O_RDONLY) {
        lfsOpenFlag |= LFS_O_RDONLY;
    }

    return lfsOpenFlag;
}

static S32 OsLfsErrno(S32 result)
{
    return (result < 0) ? -result : result;
}

static void OsLfsConfigAdapter(struct PartitionCfg *pCfg, struct lfs_config *lfsCfg)
{
    lfsCfg->context = (void *)pCfg->partNo;

    lfsCfg->read_size = pCfg->readSize;
    lfsCfg->prog_size = pCfg->writeSize;
    lfsCfg->cache_size = pCfg->cacheSize;
    lfsCfg->block_cycles = pCfg->blockCycles;
    lfsCfg->lookahead_size = pCfg->lookaheadSize;
    lfsCfg->block_size = pCfg->blockSize;
    lfsCfg->block_count = pCfg->blockCount;

    lfsCfg->read = OsLfsBlockRead;
    lfsCfg->prog = OsLfsBlockWrite;
    lfsCfg->erase = OsLfsBlockErase;
    lfsCfg->sync = OsLfsBlockSync;

    g_partitionCfg.readFunc = pCfg->readFunc;
    g_partitionCfg.writeFunc = pCfg->writeFunc;
    g_partitionCfg.eraseFunc = pCfg->eraseFunc;
}

static S32 OsLfsMount(struct TagMountPoint *mp, uintptr_t mountflags, const void *data)
{
    S32 ret;
    S32 size;
    lfs_t *mountHdl = NULL;
    struct lfs_config *cfg = NULL;

    if ((mp == NULL) || (mp->mPath == NULL) || (data == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    size = sizeof(lfs_t) + sizeof(struct lfs_config);
    mountHdl = (lfs_t *)malloc(size);
    if (mountHdl == NULL) {
        errno = ENODEV;
        return FS_NOK;
    }
    if (memset_s(mountHdl, size, 0, size) != EOK) {
        free(mountHdl);
        errno = EBADF;
        return FS_NOK;
    }
    mp->mData = (void *)mountHdl;
    cfg = (struct lfs_config *)((uintptr_t)mountHdl + sizeof(lfs_t));

    OsLfsConfigAdapter((struct PartitionCfg *)data, cfg);

    ret = lfs_mount((lfs_t *)mp->mData, cfg);
    if (ret != 0) {
        ret = lfs_format((lfs_t *)mp->mData, cfg);
        if (ret == 0) {
            ret = lfs_mount((lfs_t *)mp->mData, cfg);
        }
    }
    if (ret != 0) {
        free(mountHdl);
        errno = OsLfsErrno(ret);
        return FS_NOK;
    }
    return ret;
}

static S32 OsLfsUmount(struct TagMountPoint *mp)
{
    S32 ret;

    if (mp == NULL) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return FS_NOK;
    }

    ret = lfs_unmount((lfs_t *)mp->mData);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    free(mp->mData);
    mp->mData = NULL;
    return ret;
}

static S32 OsLfsUnlink(struct TagMountPoint *mp, const char *fileName)
{
    S32 ret;

    if ((mp == NULL) || (fileName == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return FS_NOK;
    }

    ret = lfs_remove((lfs_t *)mp->mData, fileName);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    return ret;
}

static S32 OsLfsMkdir(struct TagMountPoint *mp, const char *dirName)
{
    S32 ret;

    if ((dirName == NULL) || (mp == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return FS_NOK;
    }

    lfs_t *lfs = (lfs_t *)mp->mData;

    ret = lfs_mkdir(lfs, dirName);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    return ret;
}

static S32 OsLfsRmdir(struct TagMountPoint *mp, const char *dirName)
{
    S32 ret;
    lfs_t *lfs = NULL;

    if (mp == NULL) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return FS_NOK;
    }

    if (dirName == NULL) {
        errno = EFAULT;
        return FS_NOK;
    }

    lfs = (lfs_t *)mp->mData;
    ret = lfs_remove(lfs, dirName);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }
    return ret;
}

static S32 OsLfsOpendir(struct TagDir *dir, const char *dirName)
{
    S32 ret;

    if ((dir == NULL) || (dir->dMp == NULL) || (dir->dMp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    lfs_t *lfs = (lfs_t *)dir->dMp->mData;
    lfs_dir_t *dirInfo = (lfs_dir_t *)malloc(sizeof(lfs_dir_t));
    if (dirInfo == NULL) {
        errno = ENOMEM;
        return FS_NOK;
    }
    if (memset_s(dirInfo, sizeof(lfs_dir_t), 0, sizeof(lfs_dir_t)) != EOK) {
        free(dirInfo);
        errno = EBADF;
        return FS_NOK;
    }

    ret = lfs_dir_open(lfs, dirInfo, dirName);
    if (ret != 0) {
        free(dirInfo);
        errno = OsLfsErrno(ret);
        return FS_NOK;
    }

    dir->dData = dirInfo;
    dir->dOffset = 0;
    return FS_OK;
}

static S32 OsLfsReaddir(struct TagDir *dir, struct dirent *dent)
{
    S32 ret;
    struct lfs_info lfsInfo;

    if ((dir == NULL) || (dir->dMp == NULL) || (dir->dMp->mData == NULL) ||
        (dent == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (dir->dData == NULL) {
        errno = EBADF;
        return FS_NOK;
    }

    lfs_t *lfs = (lfs_t *)dir->dMp->mData;
    lfs_dir_t *dirInfo = (lfs_dir_t *)dir->dData;

    ret = lfs_dir_read(lfs, dirInfo, &lfsInfo);
    if (ret == TRUE) {
        pthread_mutex_lock(&g_fsLocalMutex);
        (void)strncpy_s(dent->d_name, sizeof(dent->d_name), lfsInfo.name, strlen(lfsInfo.name) + 1);
        if (lfsInfo.type == LFS_TYPE_DIR) {
            dent->d_type = DT_DIR;
        } else if (lfsInfo.type == LFS_TYPE_REG) {
            dent->d_type = DT_REG;
        }

        dent->d_reclen = lfsInfo.size;
        pthread_mutex_unlock(&g_fsLocalMutex);

        return FS_OK;
    }

    if (ret != 0) {
        errno = OsLfsErrno(ret);
    }

    return FS_NOK;
}

static S32 OsLfsClosedir(struct TagDir *dir)
{
    S32 ret;

    if ((dir == NULL) || (dir->dMp == NULL) || (dir->dMp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (dir->dData == NULL) {
        errno = EBADF;
        return FS_NOK;
    }

    lfs_t *lfs = (lfs_t *)dir->dMp->mData;
    lfs_dir_t *dirInfo = (lfs_dir_t *)dir->dData;

    ret = lfs_dir_close(lfs, dirInfo);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    free(dirInfo);
    dir->dData = NULL;

    return ret;
}

static S32 OsLfsOpen(struct TagFile *file, const char *pathName, S32 openFlag)
{
    S32 ret;
    lfs_file_t *lfsHandle = NULL;

    if ((pathName == NULL) || (file == NULL) || (file->fMp == NULL) ||
        (file->fMp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    lfsHandle = (lfs_file_t *)malloc(sizeof(lfs_file_t));
    if (lfsHandle == NULL) {
        errno = ENOMEM;
        return FS_NOK;
    }

    S32 lfsOpenFlag = OsConvertFlagToLfsOpenFlag(openFlag);
    ret = lfs_file_open((lfs_t *)file->fMp->mData, lfsHandle, pathName, lfsOpenFlag);
    if (ret != 0) {
        free(lfsHandle);
        errno = OsLfsErrno(ret);
        return INVALID_FD;
    }

    file->fData = (void *)lfsHandle;
    return ret;
}

static S32 OsLfsRead(struct TagFile *file, char *buf, size_t len)
{
    S32 ret;
    struct TagMountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if (buf == NULL) {
        errno = EFAULT;
        return FS_NOK;
    }

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return FS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    ret = lfs_file_read((lfs_t *)mp->mData, lfsHandle, buf, len);
    if (ret < 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }
    return ret;
}

static S32 OsLfsWrite(struct TagFile *file, const char *buf, size_t len)
{
    S32 ret;
    struct TagMountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if (buf == NULL) {
        errno = EFAULT;
        return FS_NOK;
    }

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return FS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    ret = lfs_file_write((lfs_t *)mp->mData, lfsHandle, buf, len);
    if (ret < 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }
    return ret;
}

static off_t OsLfsSeek(struct TagFile *file, off_t offset, S32 whence)
{
    off_t ret;
    struct TagMountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return (off_t)FS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return (off_t)FS_NOK;
    }

    ret = (off_t)lfs_file_seek((lfs_t *)mp->mData, lfsHandle, offset, whence);
    if (ret < 0) {
        errno = OsLfsErrno(ret);
        ret = (off_t)FS_NOK;
    }

    return ret;
}

static S32 OsLfsClose(struct TagFile *file)
{
    S32 ret;
    struct TagMountPoint *mp = NULL;
    lfs_file_t *lfsHandle = NULL;

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return FS_NOK;
    }

    lfsHandle = (lfs_file_t *)file->fData;
    mp = file->fMp;
    if ((mp == NULL) || (mp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    pthread_mutex_lock(&g_fsLocalMutex);
    ret = lfs_file_close((lfs_t *)mp->mData, lfsHandle);
    pthread_mutex_unlock(&g_fsLocalMutex);

    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    free(file->fData);
    file->fData = NULL;
    return ret;
}

static S32 OsLfsRename(struct TagMountPoint *mp, const char *oldName, const char *newName)
{
    S32 ret;

    if ((mp == NULL) || (oldName == NULL) || (newName == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return FS_NOK;
    }

    ret = lfs_rename((lfs_t *)mp->mData, oldName, newName);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    return ret;
}

static S32 OsLfsStat(struct TagMountPoint *mp, const char *path, struct stat *buf)
{
    S32 ret;
    struct lfs_info info;

    if ((mp == NULL) || (path == NULL) || (buf == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    if (mp->mData == NULL) {
        errno = ENOENT;
        return FS_NOK;
    }

    ret = lfs_stat((lfs_t *)mp->mData, path, &info);
    if (ret == 0) {
        buf->st_size = info.size;
        if (info.type == LFS_TYPE_REG) {
            buf->st_mode = S_IFREG;
        } else {
            buf->st_mode = S_IFDIR;
        }
    } else {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }

    return ret;
}

static S32 OsLfsSync(struct TagFile *file)
{
    S32 ret;
    struct TagMountPoint *mp = NULL;

    if ((file == NULL) || (file->fData == NULL)) {
        errno = EBADF;
        return FS_NOK;
    }

    if ((file->fMp == NULL) || (file->fMp->mData == NULL)) {
        errno = EFAULT;
        return FS_NOK;
    }

    mp = file->fMp;
    ret = lfs_file_sync((lfs_t *)mp->mData, (lfs_file_t *)file->fData);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }
    return ret;
}

static S32 OsLfsFormat(const char *partName, void *privData)
{
    S32 ret;
    lfs_t lfs = {0};
    struct lfs_config cfg = {0};

    (void)partName;

    OsLfsConfigAdapter((struct PartitionCfg *)privData, &cfg);

    ret = lfs_format(&lfs, &cfg);
    if (ret != 0) {
        errno = OsLfsErrno(ret);
        ret = FS_NOK;
    }
    return ret;
}

static struct TagMountOps g_lfsMnt = {
    .mount = OsLfsMount,
    .umount = OsLfsUmount,
    .umount2 = NULL,
    .statfs = NULL,
};

static struct TagFileOps g_lfsFops = {
    .open = OsLfsOpen,
    .close = OsLfsClose,
    .read = OsLfsRead,
    .write = OsLfsWrite,
    .lseek = OsLfsSeek,
    .stat = OsLfsStat,
    .truncate = NULL,
    .unlink = OsLfsUnlink,
    .rename = OsLfsRename,
    .ioctl = NULL, /* 不支持 */
    .sync = OsLfsSync,
    .rmdir = OsLfsRmdir,
    .opendir = OsLfsOpendir,
    .readdir = OsLfsReaddir,
    .closedir = OsLfsClosedir,
    .mkdir = OsLfsMkdir,
};

static struct TagFsManagement g_lfsMgt = {
    .fdisk = NULL,
    .format = OsLfsFormat,
};

void OsLfsInit(void)
{
    (void)OsFsRegister("littlefs", &g_lfsMnt, &g_lfsFops, &g_lfsMgt);
}
