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
#include "stdlib.h"
#include "string.h"
#include "securec.h"
#include "vfs_partition.h"
#include "vfs_operations.h"
#include "prt_fs.h"
#include "vfs_maps.h"
#include "vfs_mount.h"

static struct TagDeviceDesc *g_deviceList;

S32 OsGetPartIdByPartName(const char *partName)
{
    if (partName == NULL) {
        return FS_NOK;
    }

    /* p 字符后面是 partId */
    char *p = strrchr(partName, 'p');
    if (p + 1 != NULL) {
        return atoi(p + 1);
    }

    return FS_NOK;
}

S32 OsGetDevIdByDevName(const char *dev)
{
    if (dev == NULL) {
        return FS_NOK;
    }

    /* dev后面是 deviceId */
    char *p = (char *)dev + strlen(dev) - 1;
    if (p != NULL) {
        return atoi(p);
    }

    return FS_NOK;
}

struct TagDeviceDesc *OsGetDeviceList(void)
{
    return g_deviceList;
}

static void OsFreeDeviceDesc(struct TagDeviceDesc *prev)
{
    if (prev == NULL) {
        return;
    }
    if (prev->dDev != NULL) {
        free((void *)prev->dDev);
    }
    if (prev->dFsType != NULL) {
        free((void *)prev->dFsType);
    }
    if (prev->dAddrArray != NULL) {
        free((void *)prev->dAddrArray);
    }
    if (prev->dLengthArray != NULL) {
        free((void *)prev->dLengthArray);
    }
    free(prev);
}

static S32 OsAddDevice(const char *dev, const char *fsType, S32 *lengthArray,
                                   S32 *addrArray, S32 partNum)
{
    struct TagDeviceDesc *prev = NULL;
    for (prev = g_deviceList; prev != NULL; prev = prev->dNext) {
        if (strcmp(prev->dDev, dev) == 0) {
            errno = -EEXIST;
            return FS_NOK;
        }
    }

    if (addrArray == NULL) {
        errno = -EFAULT;
        return FS_NOK;
    }

    prev = (struct TagDeviceDesc *)malloc(sizeof(struct TagDeviceDesc));
    if (prev == NULL) {
        errno = -ENOMEM;
        return FS_NOK;
    }
    prev->dDev = strdup(dev);
    prev->dFsType  = strdup(fsType);
    prev->dAddrArray = (S32 *)malloc(partNum * sizeof(S32));
    if (prev->dDev == NULL || prev->dFsType == NULL || prev->dAddrArray == NULL) {
        OsFreeDeviceDesc(prev);
        errno = -ENOMEM;
        return FS_NOK;
    }
    (void)memcpy_s(prev->dAddrArray, partNum * sizeof(S32), addrArray, partNum * sizeof(S32));

    if (lengthArray != NULL) {
        prev->dLengthArray = (S32 *)malloc(partNum * sizeof(S32));
        if (prev->dLengthArray == NULL) {
            OsFreeDeviceDesc(prev);
            errno = -ENOMEM;
            return FS_NOK;
        }
        (void)memcpy_s(prev->dLengthArray, partNum * sizeof(S32), lengthArray, partNum * sizeof(S32));
    }

    prev->dNext = g_deviceList;
    prev->dPartNum = partNum;
    g_deviceList = prev;
    return FS_OK;
}


S32 PRT_DiskPartition(const char *dev, const char *fsType, S32 *lengthArray,
                      S32 *addrArray, S32 partNum)
{
    S32 ret = OsVfsFsMgtDisk(dev, fsType, lengthArray, partNum);
    if (ret != FS_OK) {
        return ret;
    }

    return OsAddDevice(dev, fsType, lengthArray, addrArray, partNum);
}

S32 PRT_PartitionFormat(const char *partName, char *fsType, void *data)
{
    S32 ret = OsVfsFindMountPoint(fsType);
    if (ret == FS_OK) {
        errno = EBUSY;
        return FS_NOK;
    }

    return OsVfsFsMgtFormat(partName, fsType, data);
}
