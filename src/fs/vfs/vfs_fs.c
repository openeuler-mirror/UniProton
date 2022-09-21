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
 * Description: 文件系统vfs层
 */

#define _GNU_SOURCE 1
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/uio.h>
#include <pthread.h>
#include "errno.h"
#include "fcntl.h"
#include "limits.h"
#include "securec.h"
#include "prt_fs.h"
#include "vfs_config.h"
#include "vfs_files.h"
#include "vfs_maps.h"
#include "vfs_mount.h"
#include "vfs_operations.h"
#include "prt_task.h"

#define FREE_AND_SET_NULL(ptr) do { \
    free(ptr);                      \
    ptr = NULL;                     \
} while (0)

static pthread_mutex_t g_vfsMutex = PTHREAD_MUTEX_INITIALIZER;

S32 OsVfsLock(void)
{
    S32 ret = pthread_mutex_lock(&g_vfsMutex);
    if (ret != 0) {
        return FS_NOK;
    }
    return FS_OK;
}

void OsVfsUnlock(void)
{
    (void)pthread_mutex_unlock(&g_vfsMutex);
    return;
}

S32 OsVfsOpen(const char *path, S32 flags, va_list ap)
{
    (void)ap;
    struct TagFile *file = NULL;
    S32 fd = -1;
    const char *pathInMp = NULL;
    struct TagMountPoint *mp = NULL;
    TskHandle taskId = 0;

    if ((path == NULL) || (path[strlen(path) - 1] == '/')) {
        VFS_ERRNO_SET(EINVAL);
        return fd;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return fd;
    }

    mp = OsVfsFindMp(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0') ||
        (mp->mFs->fsFops == NULL) || (mp->mFs->fsFops->open == NULL)) {
        VFS_ERRNO_SET(ENOENT);
        OsVfsUnlock();
        return fd;
    }

    if ((mp->mWriteEnable == FALSE) &&
        (flags & (O_CREAT | O_WRONLY | O_RDWR))) {
        VFS_ERRNO_SET(EACCES);
        OsVfsUnlock();
        return fd;
    }

    file = OsVfsGetFile();
    if (file == NULL) {
        VFS_ERRNO_SET(ENFILE);
        OsVfsUnlock();
        return fd;
    }

    file->fullPath = strdup(path);
    if (file->fullPath == NULL) {
        VFS_ERRNO_SET(ENOMEM);
        OsVfsPutFile(file);
        OsVfsUnlock();
        return FS_NOK;
    }

    (void)PRT_TaskSelf(&taskId);
    file->fFlags = (U32)flags;
    file->fOffset = 0;
    file->fData = NULL;
    file->fFops = mp->mFs->fsFops;
    file->fMp = mp;
    file->fOwner = taskId;

    if (file->fFops->open(file, pathInMp, flags) == 0) {
        mp->mRefs++;
        fd = OsFileToFd(file);
        file->fStatus = FILE_STATUS_READY;
    } else {
        OsVfsPutFile(file);
    }

    OsVfsUnlock();
    return fd;
}

static struct TagFile *OsVfsAttachFile(S32 fd, U32 status)
{
    struct TagFile *file = NULL;

    if ((fd < 0) || (fd >= CONFIG_NFILE_DESCRIPTORS)) {
        VFS_ERRNO_SET(EBADF);
        return NULL;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EFAULT);
        return NULL;
    }

    file = OsFdToFile(fd);
    if ((file == NULL) || (file->fMp == NULL)) {
        VFS_ERRNO_SET(EBADF);
        OsVfsUnlock();
        return NULL;
    }

    if (file->fStatus != FILE_STATUS_READY) {
        VFS_ERRNO_SET(EBADF);
        OsVfsUnlock();
        return NULL;
    }

    file->fStatus = status;
    return file;
}

static struct TagFile *OsVfsAttachFileReady(S32 fd)
{
    return OsVfsAttachFile(fd, FILE_STATUS_READY);
}

static struct TagFile *OsVfsAttachFileWithStatus(S32 fd, S32 status)
{
    return OsVfsAttachFile(fd, (U32)status);
}

static void OsVfsDetachFile(const struct TagFile *file)
{
    (void)file;
    OsVfsUnlock();
}

S32 OsVfsClose(S32 fd)
{
    S32 ret = FS_NOK;
    struct TagFile *file = OsVfsAttachFileWithStatus(fd, FILE_STATUS_CLOSING);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFops != NULL) && (file->fFops->close != NULL)) {
        ret = file->fFops->close(file);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    if ((ret == 0) && (file->fMp != NULL)) {
        file->fMp->mRefs--;
    }

    if (file->fullPath != NULL) {
        free((void *)file->fullPath);
    }

    OsVfsDetachFile(file);
    OsVfsPutFile(file);
    return ret;
}

ssize_t OsVfsRead(S32 fd, char *buff, size_t bytes)
{
    struct TagFile *file = NULL;
    ssize_t ret = (ssize_t)-1;

    if (buff == NULL) {
        VFS_ERRNO_SET(EFAULT);
        return ret;
    }

    if (bytes == 0) {
        return 0;
    }

    file = OsVfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFlags & O_ACCMODE) == O_WRONLY) {
        VFS_ERRNO_SET(EACCES);
    } else if ((file->fFops != NULL) && (file->fFops->read != NULL)) {
        ret = file->fFops->read(file, buff, bytes);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsDetachFile(file);
    return ret;
}

ssize_t OsVfsWrite(S32 fd, const void *buff, size_t bytes)
{
    struct TagFile *file = NULL;
    ssize_t ret = (ssize_t)FS_NOK;

    if ((buff == NULL) || (bytes == 0)) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    file = OsVfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFlags & O_ACCMODE) == O_RDONLY) {
        VFS_ERRNO_SET(EACCES);
    } else if ((file->fFops != NULL) && (file->fFops->write != NULL)) {
        ret = file->fFops->write(file, buff, bytes);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsDetachFile(file);
    return ret;
}

off_t OsVfsLseek(S32 fd, off_t off, S32 whence)
{
    off_t ret = (off_t)FS_NOK;
    struct TagFile *file = OsVfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFops == NULL) || (file->fFops->lseek == NULL)) {
        ret = file->fOffset;
    } else {
        ret = file->fFops->lseek(file, off, whence);
    }

    OsVfsDetachFile(file);
    return ret;
}

S32 OsVfsStat(const char *path, struct stat *stat)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    S32 ret = FS_NOK;

    if ((path == NULL) || (stat == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return ret;
    }

    mp = OsVfsFindMp(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0')) {
        VFS_ERRNO_SET(ENOENT);
        OsVfsUnlock();
        return ret;
    }

    if (mp->mFs->fsFops->stat != NULL) {
        ret = mp->mFs->fsFops->stat(mp, pathInMp, stat);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsUnlock();
    return ret;
}

S32 OsVfsFstat(S32 fd, struct stat *buf)
{
    S32 ret;
    struct TagFile *filep = OsVfsAttachFileReady(fd);
    if ((filep == NULL) || (filep->fMp == NULL) || filep->fullPath == NULL) {
        return FS_NOK;
    }
    ret = stat(filep->fullPath, buf);
    OsVfsDetachFile(filep);
    return ret;
}

S32 OsVfsStatfs(const char *path, struct statfs *buf)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    S32 ret = FS_NOK;

    if ((path == NULL) || (buf == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return ret;
    }

    mp = OsVfsFindMp(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0')) {
        VFS_ERRNO_SET(ENOENT);
        OsVfsUnlock();
        return ret;
    }

    if (mp->mFs->fsFops->stat != NULL) {
        ret = mp->mFs->fsMops->statfs(pathInMp, buf);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsUnlock();
    return ret;
}

S32 OsVfsUnlink(const char *path)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    S32 ret = FS_NOK;

    if (path == NULL) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return ret;
    }

    mp = OsVfsFindMp(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0') ||
        (mp->mFs->fsFops->unlink == NULL)) {
        VFS_ERRNO_SET(ENOENT);
        OsVfsUnlock();
        return ret;
    }

    ret = mp->mFs->fsFops->unlink(mp, pathInMp);

    OsVfsUnlock();
    return ret;
}

S32 OsVfsRename(const char *oldName, const char *newName)
{
    struct TagMountPoint *mpOld = NULL;
    struct TagMountPoint *mpNew = NULL;
    const char *pathInMpOld = NULL;
    const char *pathInMpNew = NULL;
    S32 ret = FS_NOK;

    if ((oldName == NULL) || (newName == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return ret;
    }

    mpOld = OsVfsFindMp(oldName, &pathInMpOld);

    if (pathInMpOld == NULL) {
        VFS_ERRNO_SET(EINVAL);
        OsVfsUnlock();
        return ret;
    }

    if ((mpOld == NULL) || (*pathInMpOld == '\0') ||
        (mpOld->mFs->fsFops->unlink == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        OsVfsUnlock();
        return ret;
    }

    mpNew = OsVfsFindMp(newName, &pathInMpNew);
    if ((mpNew == NULL) || (pathInMpNew == NULL) || (*pathInMpNew == '\0') || (mpNew->mFs->fsFops->unlink == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        OsVfsUnlock();
        return ret;
    }

    if (mpOld != mpNew) {
        VFS_ERRNO_SET(EXDEV);
        OsVfsUnlock();
        return ret;
    }

    if (mpOld->mFs->fsFops->rename != NULL) {
        ret = mpOld->mFs->fsFops->rename(mpOld, pathInMpOld, pathInMpNew);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsUnlock();
    return ret;
}

S32 OsVfsIoctl(S32 fd, S32 func, unsigned long arg)
{
    S32 ret = FS_NOK;
    struct TagFile *file = OsVfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if ((file->fFops != NULL) && (file->fFops->ioctl != NULL)) {
        ret = file->fFops->ioctl(file, func, arg);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsDetachFile(file);
    return ret;
}

S32 OsVfsSync(S32 fd)
{
    S32 ret = FS_NOK;
    struct TagFile *file = OsVfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if (file->fMp->mWriteEnable == FALSE) {
        VFS_ERRNO_SET(EACCES);
        OsVfsDetachFile(file);
        return ret;
    }

    if ((file->fFops != NULL) && (file->fFops->sync != NULL)) {
        ret = file->fFops->sync(file);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsDetachFile(file);
    return ret;
}

DIR *OsVfsOpendir(const char *path)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    struct TagDir *dir = NULL;
    U32 ret;

    if (path == NULL) {
        VFS_ERRNO_SET(EINVAL);
        return NULL;
    }

    dir = (struct TagDir *)malloc(sizeof(struct TagDir));
    if (dir == NULL) {
        VFS_ERRNO_SET(ENOMEM);
        return NULL;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        free(dir);
        return NULL;
    }

    mp = OsVfsFindMp(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL)) {
        VFS_ERRNO_SET(ENOENT);
        OsVfsUnlock();
        free(dir);
        return NULL;
    }

    if (mp->mFs->fsFops->opendir == NULL) {
        VFS_ERRNO_SET(ENOTSUP);
        OsVfsUnlock();
        free(dir);
        return NULL;
    }

    dir->dMp = mp;
    dir->dOffset = 0;

    ret = (U32)mp->mFs->fsFops->opendir(dir, pathInMp);
    if (ret == 0) {
        mp->mRefs++;
    } else {
        free(dir);
        dir = NULL;
    }

    OsVfsUnlock();
    return (DIR *)dir;
}

struct dirent *OsVfsReaddir(DIR *d)
{
    struct dirent *ret = NULL;
    struct TagDir *dir = (struct TagDir *)d;

    if ((dir == NULL) || (dir->dMp == NULL)) {
        VFS_ERRNO_SET(EINVAL);
        return NULL;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return NULL;
    }

    if ((dir->dMp->mFs != NULL) && (dir->dMp->mFs->fsFops != NULL) &&
        (dir->dMp->mFs->fsFops->readdir != NULL)) {
        if (dir->dMp->mFs->fsFops->readdir(dir, &dir->dDent) == 0) {
            ret = &dir->dDent;
        }
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsUnlock();
    return ret;
}

S32 OsVfsClosedir(DIR *d)
{
    struct TagMountPoint *mp = NULL;
    S32 ret = FS_NOK;
    struct TagDir *dir = (struct TagDir *)d;

    if ((dir == NULL) || (dir->dMp == NULL)) {
        VFS_ERRNO_SET(EBADF);
        return ret;
    }

    mp = dir->dMp;

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return ret;
    }

    if ((dir->dMp->mFs != NULL) && (dir->dMp->mFs->fsFops != NULL) &&
        (dir->dMp->mFs->fsFops->closedir != NULL)) {
        ret = dir->dMp->mFs->fsFops->closedir(dir);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    if (ret == 0) {
        mp->mRefs--;
    } else {
        VFS_ERRNO_SET(EBADF);
    }

    OsVfsUnlock();
    free(dir);
    dir = NULL;
    return ret;
}

S32 OsVfsMkdir(const char *path, S32 mode)
{
    struct TagMountPoint *mp = NULL;
    const char *pathInMp = NULL;
    S32 ret = FS_NOK;
    (void)mode;

    if (path == NULL) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return ret;
    }

    mp = OsVfsFindMp(path, &pathInMp);
    if ((mp == NULL) || (pathInMp == NULL) || (*pathInMp == '\0')) {
        VFS_ERRNO_SET(ENOENT);
        OsVfsUnlock();
        return ret;
    }

    if (mp->mFs->fsFops->mkdir != NULL) {
        ret = mp->mFs->fsFops->mkdir(mp, pathInMp);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
        ret = FS_NOK;
    }

    OsVfsUnlock();
    return ret;
}

S32 OsVfsFcntl(S32 fd, S32 cmd, va_list ap)
{
    S32 ret = FS_NOK;
    U32 flags;
    struct TagFile *filep = OsVfsAttachFileReady(fd);
    if (filep == NULL) {
        return ret;
    }

    if (filep->fFops == NULL) {
        VFS_ERRNO_SET(EBADF);
        OsVfsDetachFile(filep);
        return ret;
    }

    if (cmd == F_GETFL) {
        ret = (S32)(filep->fFlags);
    } else if (cmd == F_SETFL) {
        flags = (U32)va_arg(ap, S32);
        flags &= PRT_FCNTL;
        filep->fFlags &= ~PRT_FCNTL;
        filep->fFlags |= flags;
        ret = FS_OK;
    } else {
        VFS_ERRNO_SET(ENOSYS);
    }
    OsVfsDetachFile(filep);
    va_end(ap);
    return ret;
}

ssize_t OsVfsPread(S32 fd, void *buff, size_t bytes, off_t off)
{
    off_t savepos, pos;
    ssize_t ret;

    if (buff == NULL) {
        VFS_ERRNO_SET(EFAULT);
        return (ssize_t)FS_NOK;
    }

    if (bytes == 0) {
        return 0;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return (ssize_t)FS_NOK;
    }

    savepos = lseek(fd, 0, SEEK_CUR);
    if (savepos == (off_t)-1) {
        OsVfsUnlock();
        return (ssize_t)FS_NOK;
    }

    pos = lseek(fd, off, SEEK_SET);
    if (pos == (off_t)-1) {
        OsVfsUnlock();
        return (ssize_t)FS_NOK;
    }

    ret = read(fd, buff, bytes);
    pos = lseek(fd, savepos, SEEK_SET);
    if ((pos == (off_t)-1) && (ret >= 0)) {
        OsVfsUnlock();
        return (ssize_t)FS_NOK;
    }

    OsVfsUnlock();
    return ret;
}

ssize_t OsVfsPwrite(S32 fd, const void *buff, size_t bytes, off_t off)
{
    ssize_t ret;
    off_t savepos, pos;

    if (buff == NULL) {
        VFS_ERRNO_SET(EFAULT);
        return (ssize_t)FS_NOK;
    }

    if (bytes == 0) {
        return 0;
    }

    if (OsVfsLock() != FS_OK) {
        VFS_ERRNO_SET(EAGAIN);
        return (ssize_t)FS_NOK;
    }

    savepos = lseek(fd, 0, SEEK_CUR);
    if (savepos == (off_t)-1) {
        OsVfsUnlock();
        return (ssize_t)FS_NOK;
    }

    pos = lseek(fd, off, SEEK_SET);
    if (pos == (off_t)-1) {
        OsVfsUnlock();
        return (ssize_t)FS_NOK;
    }

    ret = write(fd, buff, bytes);
    pos = lseek(fd, savepos, SEEK_SET);
    if ((pos == (off_t)-1) && (ret >= 0)) {
        OsVfsUnlock();
        return (ssize_t)FS_NOK;
    }

    OsVfsUnlock();
    return ret;
}

S32 OsVfsFtruncate(S32 fd, off_t length)
{
    S32 ret = FS_NOK;
    struct TagFile *file = NULL;

    if (length <= 0) {
        VFS_ERRNO_SET(EINVAL);
        return ret;
    }

    file = OsVfsAttachFileReady(fd);
    if (file == NULL) {
        return ret;
    }

    if (file->fMp->mWriteEnable == FALSE) {
        VFS_ERRNO_SET(EACCES);
        OsVfsDetachFile(file);
        return ret;
    }

    if ((file->fFlags & O_ACCMODE) == O_RDONLY) {
        VFS_ERRNO_SET(EACCES);
    } else if ((file->fFops != NULL) && (file->fFops->truncate != NULL)) {
        ret = file->fFops->truncate(file, length);
    } else {
        VFS_ERRNO_SET(ENOTSUP);
    }

    OsVfsDetachFile(file);
    return ret;
}

ssize_t OsVfsReadv(S32 fd, const struct iovec *iovBuf, S32 iovcnt)
{
    S32 i;
    errno_t ret;
    char *buf = NULL;
    char *curBuf = NULL;
    char *readBuf = NULL;
    size_t bufLen = 0;
    size_t bytesToRead;
    ssize_t totalBytesRead;
    size_t totalLen;
    const struct iovec *iov = (const struct iovec *)iovBuf;

    if ((iov == NULL) || (iovcnt <= 0) || (iovcnt > IOV_MAX_CNT)) {
        return (ssize_t)FS_NOK;
    }

    for (i = 0; i < iovcnt; ++i) {
        if ((SSIZE_MAX - bufLen) < iov[i].iov_len) {
            return (ssize_t)FS_NOK;
        }
        bufLen += iov[i].iov_len;
    }
    if (bufLen == 0) {
        return (ssize_t)FS_NOK;
    }
    totalLen = bufLen * sizeof(char);
    buf = (char *)malloc(totalLen);
    if (buf == NULL) {
        return (ssize_t)FS_NOK;
    }

    totalBytesRead = read(fd, buf, bufLen);
    if ((size_t)totalBytesRead < totalLen) {
        totalLen = (size_t)totalBytesRead;
    }
    curBuf = buf;
    for (i = 0; i < iovcnt; ++i) {
        readBuf = (char *)iov[i].iov_base;
        bytesToRead = iov[i].iov_len;

        size_t lenToRead = totalLen < bytesToRead ? totalLen : bytesToRead;
        ret = memcpy_s(readBuf, bytesToRead, curBuf, lenToRead);
        if (ret != EOK) {
            free(buf);
            return (ssize_t)FS_NOK;
        }
        if (totalLen < (size_t)bytesToRead) {
            break;
        }
        curBuf += bytesToRead;
        totalLen -= bytesToRead;
    }
    free(buf);
    return totalBytesRead;
}

ssize_t OsVfsWritev(S32 fd, const struct iovec *iovBuf, S32 iovcnt)
{
    S32 i;
    errno_t ret;
    char *buf = NULL;
    char *curBuf = NULL;
    char *writeBuf = NULL;
    size_t bufLen = 0;
    size_t bytesToWrite;
    ssize_t totalBytesWritten;
    size_t totalLen;
    const struct iovec *iov = iovBuf;

    if ((iov == NULL) || (iovcnt <= 0) || (iovcnt > IOV_MAX_CNT)) {
        return (ssize_t)FS_NOK;
    }

    for (i = 0; i < iovcnt; ++i) {
        if ((SSIZE_MAX - bufLen) < iov[i].iov_len) {
            VFS_ERRNO_SET(EINVAL);
            return (ssize_t)FS_NOK;
        }
        bufLen += iov[i].iov_len;
    }
    if (bufLen == 0) {
        return (ssize_t)FS_NOK;
    }
    totalLen = bufLen * sizeof(char);
    buf = (char *)malloc(totalLen);
    if (buf == NULL) {
        return (ssize_t)FS_NOK;
    }
    curBuf = buf;
    for (i = 0; i < iovcnt; ++i) {
        writeBuf = (char *)iov[i].iov_base;
        bytesToWrite = iov[i].iov_len;
        if (((ssize_t)totalLen <= 0) || ((ssize_t)bytesToWrite <= 0)) {
            continue;
        }
        ret = memcpy_s(curBuf, totalLen, writeBuf, bytesToWrite);
        if (ret != EOK) {
            free(buf);
            return (ssize_t)FS_NOK;
        }
        curBuf += bytesToWrite;
        totalLen -= bytesToWrite;
    }

    totalBytesWritten = write(fd, buf, bufLen);
    free(buf);

    return totalBytesWritten;
}
