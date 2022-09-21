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

#ifndef VFS_FILES_H
#define VFS_FILES_H

#include "dirent.h"
#include "sys/stat.h"
#include "unistd.h"
#include "prt_fs.h"

#define FILE_STATUS_NOT_USED 0
#define FILE_STATUS_INITING 1
#define FILE_STATUS_READY 2
#define FILE_STATUS_CLOSING 3

struct TagFileOps;
struct TagFile;
struct TagDir;
struct TagMountPoint;

struct TagFileOps {
    S32     (*open)(struct TagFile *, const char *, S32);
    S32     (*close)(struct TagFile *);
    ssize_t (*read)(struct TagFile *, char *, size_t);
    ssize_t (*write)(struct TagFile *, const char *, size_t);
    off_t   (*lseek)(struct TagFile *, off_t, S32);
    S32     (*stat)(struct TagMountPoint *, const char *, struct stat *);
    S32     (*truncate)(struct TagFile *, off_t);
    S32     (*unlink)(struct TagMountPoint *, const char *);
    S32     (*rename)(struct TagMountPoint *, const char *, const char *);
    S32     (*ioctl)(struct TagFile *, S32, uintptr_t);
    S32     (*sync)(struct TagFile *);
    S32     (*opendir)(struct TagDir *, const char *);
    S32     (*readdir)(struct TagDir *, struct dirent *);
    S32     (*closedir)(struct TagDir *);
    S32     (*mkdir)(struct TagMountPoint *, const char *);
    S32     (*rmdir)(struct TagMountPoint *, const char *);
};

struct TagFile {
    const struct TagFileOps *fFops;
    U32                     fFlags;
    U32                     fStatus;
    off_t                   fOffset;
    S32                     fOwner;
    struct TagMountPoint    *fMp;
    void                    *fData;
    const char              *fullPath;
};

struct TagDir {
    struct TagMountPoint *dMp;
    struct dirent        dDent;
    off_t                dOffset;
    void                 *dData;
};

S32 OsFileToFd(struct TagFile *file);
struct TagFile *OsFdToFile(S32 fd);
struct TagFile *OsVfsGetFile(void);
struct TagFile *OsVfsGetFileSpec(S32 fd);
void OsVfsPutFile(struct TagFile *file);

#endif /* VFS_FILES_H */
