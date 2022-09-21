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

#ifndef VFS_PARTITION_H
#define VFS_PARTITION_H

#include "prt_typedef.h"

#define MAX_PARTITION_NUM 4

S32 OsGetPartIdByPartName(const char *partName);
S32 OsGetDevIdByDevName(const char *dev);
struct TagDeviceDesc *OsGetDeviceList(void);

struct TagDeviceDesc {
    struct TagFsMap      *dFs;             /* 文件系统信息 */
    struct TagDeviceDesc *dNext;           /* 指向下一个挂载节点 */
    const char           *dPath;           /* 挂载节点路径, /system, /usr, etc. */
    const char           *dDev;            /* 设备名, "emmc0p0", "emmc0p1", etc. */
    void                 *dData;           /* 挂载节点数据 */
    const char           *dFsType;         /* 文件系统类型 */
    S32                  *dLengthArray;    /* 设备分区长度记录数组 */
    uintptr_t            *dAddrArray;      /* 设备分区地址记录数组 */
    S32                  dPartNum;         /* 设备分区号 */
};

#endif /* VFS_PARTITION_H */
