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
 * Description: 提供公共的异常处理机制(如PRT_SysReboot)
 */
#include "prt_kexc_external.h"
#include "prt_exc_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_err_external.h"

OS_SEC_L4_BSS struct ExcModInfo g_excModInfo;

#if !defined(OS_OPTION_SMP)
// 若作为KOS异常使用,因为UOS异常可恢复异常直接清除该变量值，而不可恢复异常直接Kill该进程了之前也会清除该变量。
OS_SEC_DATA U32 g_curNestCount = 0;
OS_SEC_BSS struct TagExcInfoInternal g_excInfoInternal;
#else
OS_SEC_BSS U32 g_curNestCount[OS_MAX_CORE_NUM];

OS_SEC_BSS struct TagExcInfoInternal g_excInfoInternal[OS_MAX_CORE_NUM];
#endif
/*
 * 描述：异常处理用户钩子函数注册
 */
OS_SEC_L4_TEXT U32 PRT_ExcRegHook(ExcProcFunc hook)
{
    uintptr_t intSave;

    if (hook == NULL) {
        return OS_ERRNO_EXC_REG_HOOK_PTR_NULL;
    }

    intSave = OsIntLock();

    g_excModInfo.excepHook = hook;

    OsIntRestore(intSave);

    return OS_OK;
}

OS_SEC_L4_TEXT void PRT_SysReboot(void)
{
#if defined(OS_OPTION_POWEROFF)
    OsCpuPowerOff();
#endif

    while (1) {
        /* Wait for HWWDG to reboot board. */
    }
}
