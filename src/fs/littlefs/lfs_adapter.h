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
 * Description: littlefs适配层代码
 */

#ifndef LFS_ADAPTER_H
#define LFS_ADAPTER_H

#include "fcntl.h"
#include "sys/stat.h"

#include "dirent.h"
#include "errno.h"
#include "vfs_operations.h"
#include "lfs.h"
#include "lfs_conf.h"
#include "lfs_util.h"
#include "memory.h"
#include "pthread.h"

#define INVALID_DEVICE_ADDR (uintptr_t)(-1)
#define INVALID_FD (-1)
void OsLfsInit(void);
#endif /* LFS_ADAPTER_H */
