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
 * Description: fs对外头文件
 */
#ifndef PRT_FS_H
#define PRT_FS_H

#include "dirent.h"
#include "sys/mount.h"
#include "sys/statfs.h"
#include "sys/stat.h"
#include "sys/uio.h"
#include "unistd.h"

#include "prt_config.h"
#include "prt_typedef.h"
#include "prt_sem.h"

struct PartitionCfg {
    /* partition low-level read func */
    S32  (*readFunc)(S32 partition, uintptr_t offset, void *buf, U32 size);
    /* partition low-level write func */
    S32  (*writeFunc)(S32 partition, uintptr_t offset, const void *buf, U32 size);
    /* partition low-level erase func */
    S32  (*eraseFunc)(S32 partition, uintptr_t offset, U32 size);

    S32 readSize;       /* size of a block read */
    S32 writeSize;      /* size of a block write */
    S32 blockSize;      /* size of an erasable block */
    S32 blockCount;     /* number of partition blocks */
    S32 cacheSize;      /* size of block caches */

    S32 partNo;         /* partition number */
    S32 lookaheadSize;  /* lookahead size */
    S32 blockCycles;    /* block cycles */
};

/*
 * @brief Divide the device into partitions.
 *
 * @param dev Device name, which customized by caller, it is recommended to
 * name it as: "emmc0", "emmc1", "flash0", etc. The name is combined with
 * "device_type" + "device_id", and the last character is device_id.
 * device_id >= 0 && device_id <= 9.
 * @param fsType Filesystem type.
 * @param lengthArray List of partition size. For example:
 *     [0x10000000, 0x2000000], 256M and 512M partitions will be created and
 *     left all will not allocated.
 * @param addrArray List of partition start addr, partition num same as lengthArray
 * @param partNum Length of 'lengthArray'.
 *
 * @return Return PRT_NOK if error. Return FS_OK if success.
 * Partition naming rules:
 *     In the current vfs, after successfully calling the 'fdisk' hook,
 *     "partNum" partitions will be created.
 *     The partition naming rules is:
 *         The name is a combination of: 'deviceName'+'p'+'partitionId',
 *         such as "emmc0p0", "emmc0p1", "emmc0p2"...
 */
S32 PRT_DiskPartition(const char *dev, const char *fsType, S32 *lengthArray, S32 *addrArray,
                      S32 partNum);

/*
 * @brief Format a partition.
 *
 * @param partName PartitionName, following the rule of fdisk hook.
 * @param data For FatFs, the data indicates a pointer to a byte which
 * specifies combination of FAT type flags, FM_FAT, FM_FAT32, FM_EXFAT and
 * bitwise-or of these three, FM_ANY.
 *
 * @return Return PRT_NOK if error. Return FS_OK if success.
 */
S32 PRT_PartitionFormat(const char *partName, char *fsType, void *data);

/*
 * @brief vfs init
 */
S32 PRT_VfsInit(void);

#endif /* PRT_FS_H */
