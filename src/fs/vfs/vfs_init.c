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
#include "prt_typedef.h"
#include "vfs_operations.h"
#if (OS_SUPPORT_LITTLEFS == YES)
#include "lfs_adapter.h"
#endif
#include "prt_fs.h"

S32 PRT_VfsInit(void)
{
#if (OS_SUPPORT_LITTLEFS == YES)
    OsLfsInit();
#endif
    return FS_OK;
}
