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

#ifndef VFS_MAPS_H
#define VFS_MAPS_H

#include "prt_fs.h"

struct TagMountOps;

struct TagFsManagement {
    S32 (*fdisk)(const char *dev, S32 *lengthArray, S32 partNum);
    S32 (*format)(const char *partName, void *data);
};

struct TagFsMap {
    const char                   *fsType;
    const struct TagMountOps     *fsMops;
    const struct TagFileOps      *fsFops;
    const struct TagFsManagement *fsMgt;
    U32                          fsRefs;
    struct TagFsMap              *next;
};

S32 OsFsRegister(const char *fsType, struct TagMountOps *fsMops,
                 struct TagFileOps *fsFops, struct TagFsManagement *fsMgt);
struct TagFsMap *OsVfsGetFsMap(const char *fsType);
S32 OsVfsFsMgtDisk(const char *dev, const char *fsType, S32 *lengthArray, S32 partNum);
S32 OsVfsFsMgtFormat(const char *partName, char *fsType, void *data);
#endif /* VFS_MAPS_H */
