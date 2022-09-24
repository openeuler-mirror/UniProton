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

#include <stdlib.h>
#include "securec.h"
#include "vfs_maps.h"
#include "vfs_operations.h"
#include "prt_fs.h"

static struct TagFsMap *g_fsMap;

struct TagFsMap *OsVfsGetFsMap(const char *fsType)
{
    struct TagFsMap *curr = g_fsMap;
    while (curr != NULL) {
        if ((curr->fsType != NULL) && (fsType != NULL) &&
            (strcmp(curr->fsType, fsType) == 0)) {
            return curr;
        }
        curr = curr->next;
    }
    return NULL;
}

S32 OsVfsFsMgtDisk(const char *dev, const char *fsType, S32 *lengthArray, S32 partNum)
{
    S32 ret = FS_OK;
    (void)OsVfsLock();
    struct TagFsMap *fMap = OsVfsGetFsMap(fsType);
    if ((fMap != NULL) && (fMap->fsMgt != NULL) && (fMap->fsMgt->fdisk != NULL)) {
        ret = fMap->fsMgt->fdisk(dev, lengthArray, partNum);
    }
    (void)OsVfsUnlock();
    return ret;
}

S32 OsVfsFsMgtFormat(const char *partName, char *fsType, void *data)
{
    S32 ret = FS_OK;
    (void)OsVfsLock();
    struct TagFsMap *fMap = OsVfsGetFsMap(fsType);
    if ((fMap != NULL) && (fMap->fsMgt != NULL) && (fMap->fsMgt->format != NULL)) {
        ret = fMap->fsMgt->format(partName, data);
    }
    (void)OsVfsUnlock();
    return ret;
}

S32 OsFsRegister(const char *fsType, struct TagMountOps *fsMops,
                             struct TagFileOps *fsFops, struct TagFsManagement *fsMgt)
{
    if ((fsMops == NULL) || (fsFops == NULL)) {
        return FS_NOK;
    }

    struct TagFsMap *newfs = (struct TagFsMap *)malloc(sizeof(struct TagFsMap));
    if (newfs == NULL) {
        return FS_NOK;
    }
    if (memset_s(newfs, sizeof(struct TagFsMap), 0, sizeof(struct TagFsMap)) != EOK) {
        free(newfs);
        return FS_NOK;
    }

    newfs->fsType = strdup(fsType);
    if (newfs->fsType == NULL) {
        free(newfs);
        return FS_NOK;
    }

    newfs->fsMops = fsMops;
    newfs->fsFops = fsFops;
    newfs->fsMgt = fsMgt;
    newfs->fsRefs = 0;

    (void)OsVfsLock();
    newfs->next = g_fsMap;
    g_fsMap = newfs;

    OsVfsUnlock();
    return FS_OK;
}
