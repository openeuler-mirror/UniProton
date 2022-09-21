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

#ifndef LFS_CONF_H
#define LFS_CONF_H

#define LITTLE_FS_STANDARD_NAME_LENGTH 50
#define LITTLE_FS_MAX_NAME_LEN 255

#define MAX_DEF_BUF_NUM 21
#define MAX_WRITE_FILE_LEN 500
#define MAX_READ_FILE_LEN 500
#define LITTLEFS_MAX_LFN_LEN 255

#ifndef LFS_MAX_OPEN_DIRS
#define LFS_MAX_OPEN_DIRS 10
#endif

#ifndef OS_LFS_MAX_OPEN_FILES
#define OS_LFS_MAX_OPEN_FILES   32
#endif

#ifndef OS_LFS_MAX_MOUNT_SIZE
#define OS_LFS_MAX_MOUNT_SIZE   3
#endif

#endif // LFS_CONF_H
