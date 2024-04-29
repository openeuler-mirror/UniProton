/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-24
 * Description: 模块加载内部头文件
 */

#ifndef PRT_MODULE_EXTERNAL_H
#define PRT_MODULE_EXTERNAL_H

#include "prt_typedef.h"
#include "prt_sys.h"
#include "prt_lib_external.h"
#include "prt_cpu_external.h"

typedef U32(*ModuleInitHook)(void);

extern ModuleInitHook g_hwiSecondaryInitHook;
extern ModuleInitHook g_cacheDfxSecondaryInitHook;
extern ModuleInitHook g_mmuSecondaryInitHook;
extern ModuleInitHook g_soupSecondaryInitHook;

/*
* 描述：设置从核中断初始化处理函数
*/
OS_SEC_ALW_INLINE INLINE void OsHwiSetSecondaryInitHook(ModuleInitHook hook)
{
    g_hwiSecondaryInitHook = hook;
}

/*
* 描述：设置从核cache初始化处理函数
*/
OS_SEC_ALW_INLINE INLINE void OsCacheDfxSetSecondaryInitHook(ModuleInitHook hook)
{
    g_cacheDfxSecondaryInitHook = hook;
}

/*
* 描述：设置从核mmu初始化处理函数
*/
OS_SEC_ALW_INLINE INLINE void OsMmuSetSecondaryInitHook(ModuleInitHook hook)
{
    g_mmuSecondaryInitHook = hook;
}

/*
* 描述：设置从核soup初始化处理函数
*/
OS_SEC_ALW_INLINE INLINE void OsSoupSetSecondaryInitHook(ModuleInitHook hook)
{
    g_soupSecondaryInitHook = hook;
}
#endif