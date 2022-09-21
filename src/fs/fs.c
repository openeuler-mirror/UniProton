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
 * Description: fs适配层代码
 */

#define _GNU_SOURCE

#include <errno.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/mount.h>
#include <sys/statfs.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/uio.h>
#include "vfs_config.h"
#include "vfs_operations.h"
#include "vfs_maps.h"
#include "vfs_mount.h"

int open(const char *path, int oflag, ...)
{
    S32 ret;
    va_list ap;
    va_start(ap, oflag);
    ret = OsVfsOpen(path, oflag, ap);
    va_end(ap);
    return OsMapToPosixRet(ret);
}

int close(int fd)
{
    S32 ret = OsVfsClose(fd);
    return OsMapToPosixRet(ret);
}

ssize_t read(int fd, void *buf, size_t nbyte)
{
   ssize_t ret = OsVfsRead(fd, buf, nbyte);
   return OsMapToPosixRet(ret);
}

ssize_t write(int fd, const void *buf, size_t nbyte)
{
    ssize_t ret = OsVfsWrite(fd, buf, nbyte);
    return OsMapToPosixRet(ret);
}

off_t lseek(int fd, off_t offset, int whence)
{
    return OsVfsLseek(fd, offset, whence);
}

int stat(const char *path, struct stat *buf)
{
    S32 ret = OsVfsStat(path, buf);
    return OsMapToPosixRet(ret);
}

int statfs(const char *path, struct statfs *buf)
{
    S32 ret = OsVfsStatfs(path, buf);
    return OsMapToPosixRet(ret);
}

int unlink(const char *path)
{
    S32 ret = OsVfsUnlink(path);
    return OsMapToPosixRet(ret);
}

int rename(const char *oldName, const char *newName)
{
    S32 ret = OsVfsRename(oldName, newName);
    return OsMapToPosixRet(ret);
}

int fsync(int fd)
{
    S32 ret = OsVfsSync(fd);
    return OsMapToPosixRet(ret);
}

DIR *opendir(const char *dirName)
{
    return OsVfsOpendir(dirName);
}

struct dirent *readdir(DIR *dir)
{
    return OsVfsReaddir(dir);
}

int closedir(DIR *dir)
{
    S32 ret = OsVfsClosedir(dir);
    return OsMapToPosixRet(ret);
}

int mkdir(const char *path, mode_t mode)
{
    S32 ret = OsVfsMkdir(path, (S32)mode);
    return OsMapToPosixRet(ret);
}

int rmdir(const char *path)
{
    S32 ret = OsVfsUnlink(path);
    return OsMapToPosixRet(ret);
}

int fstat(int fd, struct stat *buf)
{
    return OsVfsFstat(fd, buf);
}

int fcntl(int fd, int cmd, ...)
{
    S32 ret;
    va_list ap;
    va_start(ap, cmd);
    ret = OsVfsFcntl(fd, cmd, ap);
    va_end(ap);
    return ret;
}

int ioctl(int fd, int request, ...)
{
    unsigned long arg;
    S32 ret;
    va_list ap;
    va_start(ap, request);
    arg = va_arg(ap, unsigned long);
    va_end(ap);
    ret = OsVfsIoctl(fd, request, arg);
    return ret;
}

ssize_t readv(int fd, const struct iovec *iovBuf, int iovcnt)
{
    return OsVfsReadv(fd, iovBuf, iovcnt);
}

ssize_t writev(int fd, const struct iovec *iovBuf, int iovcnt)
{
    return OsVfsWritev(fd, iovBuf, iovcnt);
}

int isatty(int fd)
{
    (void)fd;
    return 0;
}

int access(const char *path, int mode)
{
    struct stat st;

    if (stat(path, &st) < 0) {
        return -1;
    }
    if ((st.st_mode & S_IFDIR) || (st.st_mode & S_IFREG)) {
        return 0;
    }
    if ((mode & W_OK) && !(st.st_mode & S_IWRITE)) {
        return -1;
    }

    return 0;
}

int ftruncate(int fd, off_t length)
{
    return OsVfsFtruncate(fd, length);
}

ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset)
{
    ssize_t ret = OsVfsPread(fd, buf, nbyte, offset);
    return (ssize_t)OsMapToPosixRet(ret);
}

ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset)
{
    ssize_t ret = OsVfsPwrite(fd, buf, nbyte, offset);
    return OsMapToPosixRet(ret);
}

int mount(const char *source, const char *target,
          const char *filesystemtype, unsigned long mountflags,
          const void *data)
{
    return OsVfsMount(source, target, filesystemtype, mountflags, data);
}

int umount(const char *target)
{
    return OsVfsUmount(target);
}

int umount2(const char *target, int flag)
{
    return OsVfsUmount2(target, flag);
}
