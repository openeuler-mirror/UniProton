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

#ifndef VFS_MOUNT_H
#define VFS_MOUNT_H

#include "sys/statfs.h"
#include "prt_fs.h"

struct TagFsMap;
struct TagMountPoint;

struct TagMountOps {
    S32 (*mount)(struct TagMountPoint *mp, uintptr_t mountflags, const void *data);
    S32 (*umount)(struct TagMountPoint *mp);
    S32 (*umount2)(struct TagMountPoint *mp, S32 flag);
    S32 (*statfs)(const char *path, struct statfs *buf);
};

struct TagMountPoint {
    struct TagFsMap      *mFs;         /* 系统文件系统信息 */
    struct TagMountPoint *mNext;       /* 指向下一个mount节点 */
    const char           *mPath;       /* 挂载的路径 */
    const char           *mDev;        /* 设备名, "emmc0p0", "emmc0p1", etc. */
    U32                  mRefs;        /* 挂载节点的引用计数 */
    void                 *mData;       /* 挂载节点的私有数据 */
    bool                 mWriteEnable; /* 是否允许写 */
};

S32 OsVfsMount(const char *source, const char *target,
               const char *filesystemtype, uintptr_t mountflags,
               const void *data);
S32 OsVfsUmount(const char *target);
S32 OsVfsUmount2(const char *target, S32 flag);
struct TagMountPoint *OsVfsFindMp(const char *path, const char **pathInMp);
S32 OsVfsFindMountPoint(const char *fsType);
#endif /* VFS_MOUNT_H */
