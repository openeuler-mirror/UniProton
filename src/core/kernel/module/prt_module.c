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
 * Create: 2024-01-25
 * Description: 模块加载
 */
#include "prt_module_external.h"

OS_SEC_BSS ModuleInitHook g_hwiSecondaryInitHook;
OS_SEC_BSS ModuleInitHook g_cacheDfxSecondaryInitHook;
OS_SEC_BSS ModuleInitHook g_mmuSecondaryInitHook;

/* 硬件栈越界功能初始化 */
OS_SEC_BSS ModuleInitHook g_soupSecondaryInitHook;

INIT_SEC_L4_TEXT U32 OsModuleInit(void)
{
    U32 ret;

    /* 模块多使用循环实现 */
    if (g_hwiSecondaryInitHook != NULL) {
        ret = g_hwiSecondaryInitHook();
        if (ret != OS_OK){
            return ret;
        }
    }

    if (g_cacheDfxSecondaryInitHook != NULL) {
        ret = g_cacheDfxSecondaryInitHook();
        if (ret != OS_OK){
            return ret;
        }
    }

    if (g_soupSecondaryInitHook != NULL) {
        ret = g_soupSecondaryInitHook();
        if (ret != OS_OK){
            return ret;
        }
    }

    if (g_mmuSecondaryInitHook != NULL) {
        ret = g_mmuSecondaryInitHook();
        if (ret != OS_OK){
            return ret;
        }
    }

    return OS_OK;
}