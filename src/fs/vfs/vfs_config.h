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
 * Description: vfs层配置文件
 */

#ifndef VFS_CONFIG_H
#define VFS_CONFIG_H

#include "prt_config.h"

/* 文件系统的起始fd, 0,1,2被用于 stdin,stdout,stderr */
#define MIN_START_FD 3

#if (OS_SUPPORT_LITTLEFS == YES)
#include "lfs_conf.h"
#define __LFS_NFILE OS_LFS_MAX_OPEN_FILES
#else
#define __LFS_NFILE 0
#endif

#define CONFIG_NFILE_DESCRIPTORS    (__LFS_NFILE)

#define NR_OPEN_DEFAULT CONFIG_NFILE_DESCRIPTORS

#define DEFAULT_DIR_MODE        0777
#define DEFAULT_FILE_MODE       0666
#endif
