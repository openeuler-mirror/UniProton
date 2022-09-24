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
#ifndef VFS_OPERATIONS_H
#define VFS_OPERATIONS_H

#include "errno.h"
#include "fcntl.h"
#include "dirent.h"
#include "stdint.h"
#include "stdarg.h"
#include "unistd.h"
#include "sys/mount.h"
#include "sys/stat.h"
#include "sys/statfs.h"
#include "prt_fs.h"

#define PRT_FCNTL   (O_NONBLOCK | O_NDELAY | O_APPEND | O_SYNC)
#define IOV_MAX_CNT 4

static inline void VFS_ERRNO_SET(U32 err)
{
    errno = err;
    return;
}

S32 OsVfsLock(void);
void OsVfsUnlock(void);
S32 OsVfsOpen(const char *path, S32 flags, va_list ap);
S32 OsVfsClose(S32 fd);
ssize_t OsVfsRead(S32 fd, char *buff, size_t bytes);
ssize_t OsVfsWrite(S32 fd, const void *buff, size_t bytes);
off_t OsVfsLseek(S32 fd, off_t off, S32 whence);
S32 OsVfsStat(const char *path, struct stat *stat);
S32 OsVfsStatfs(const char *path, struct statfs *buf);
S32 OsVfsFstat(S32 fd, struct stat *buf);
S32 OsVfsUnlink(const char *path);
S32 OsVfsRename(const char *oldName, const char *newName);
S32 OsVfsSync(S32 fd);
DIR *OsVfsOpendir(const char *path);
struct dirent *OsVfsReaddir(DIR *d);
S32 OsVfsClosedir(DIR *d);
S32 OsVfsMkdir(const char *path, S32 mode);
S32 OsVfsFcntl(S32 fd, S32 cmd, va_list ap);
S32 OsVfsIoctl(S32 fd, S32 func, unsigned long arg);
ssize_t OsVfsPread(S32 fd, void *buff, size_t bytes, off_t off);
ssize_t OsVfsPwrite(S32 fd, const void *buff, size_t bytes, off_t off);
S32 OsVfsFtruncate(S32 fd, off_t length);
ssize_t OsVfsReadv(S32 fd, const struct iovec *iovBuf, S32 iovcnt);
ssize_t OsVfsWritev(S32 fd, const struct iovec *iovBuf, S32 iovcnt);

static inline S32 OsMapToPosixRet(S32 ret)
{
    return ((ret) < 0 ? -1 : (ret));
}

#ifndef FS_OK
#define FS_OK   0
#endif

#ifndef FS_NOK
#define FS_NOK (-1)
#endif

#endif /* VFS_OPERATIONS_H */
