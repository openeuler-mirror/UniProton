/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: hook模块的模块内头文件
 */
#ifndef PRT_HOOK_INTERNAL_H
#define PRT_HOOK_INTERNAL_H

#include "prt_mem.h"
#include "prt_hook_external.h"
#include "prt_lib_external.h"

/*
 * 模块内宏定义
 */
#define OS_HOOK_EMPTY NULL

#define OS_IS_SHOOK_TYPE(hookType) ((U32)(hookType) >= (U32)OS_SHOOK_TYPE_START && (U32)(hookType) < (U32)OS_HOOK_TYPE_TOTAL)

#define OS_IS_MHOOK_TYPE(hookType) ((U32)(hookType) < (U32)OS_SHOOK_TYPE_START)

#define OS_IS_HOOK_TYPE(hookType)  ((U32)(hookType) < (U32)OS_HOOK_TYPE_TOTAL)

#endif /* PRT_HOOK_INTERNAL_H */
