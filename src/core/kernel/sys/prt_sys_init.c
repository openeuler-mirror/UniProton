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
 * Description: System base function implementation
 */
#include "prt_timer.h"
#include "prt_clk.h"
#include "prt_hwi.h"
#include "prt_attr_external.h"
#include "prt_sys_external.h"
#include "prt_lib_external.h"
#if defined(OS_OPTION_SMP)
#include "prt_sched_external.h"
#endif

OS_SEC_BSS U8 g_cpuType;

OS_SEC_L4_TEXT U8 OsGetCpuType(void)
{
    return g_cpuType;
}

OS_SEC_L4_TEXT U32 OsSysTimeHookReg(SysTimeFunc hook)
{
#if defined(OS_OPTION_SYS_TIME_USR)
    return OsSetSysTimeHook(hook);
#else
    (void)hook;
    return OsSetSysTimeHook(PRT_ClkGetCycleCount64);
#endif
}

OS_SEC_L4_TEXT U32 OsSysRegister(struct SysModInfo *modInfo)
{
    U32 ret;

    if (modInfo->systemClock < OS_SYS_US_PER_SECOND) {
        return OS_ERRNO_SYS_CLOCK_INVALID;
    }

#if defined(OS_OPTION_SMP)
    if ((modInfo->coreRunNum > modInfo->coreMaxNum) || (modInfo->coreRunNum == 0)) {
        return OS_ERRNO_SYS_CORE_RUNNUM_INVALID;
    }
#endif

    ret = OsSysTimeHookReg(modInfo->sysTimeHook);
    if (ret != OS_OK) {
        return ret;
    }

    UNI_FLAG = 0;
    g_systemClock = modInfo->systemClock;
    g_cpuType = (U8)modInfo->cpuType;
#if defined(OS_OPTION_SMP)
    g_numOfCores = modInfo->coreRunNum;
    g_primaryCoreId = modInfo->corePrimary;
#endif
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
    if (OsHwiCheckMaxNum(modInfo->hwiMaxNum) == FALSE) {
        return OS_ERRNO_SYS_HWI_MAX_NUM_CONFIG_INVALID;
    }
    g_hwiMaxNumConfig = modInfo->hwiMaxNum;
#endif

    return ret;
}

#if defined(OS_OPTION_SMP)
INIT_SEC_L4_TEXT void OsGetCoreStr(struct CoreNumStr *str)
{
    S32 i;
    U32 coreId = OsGetHwThreadId();

    for (i = OS_CORE_STR_NUM_INDEX; i >= 0; i--) {
        str->coreNo[i] = '0' + coreId % OS_DECIMAL;
        coreId = coreId / OS_DECIMAL;
    }
    str->coreNo[OS_CORE_STR_END_INDEX] = 0;
    return;
}
#endif
OS_SEC_L4_TEXT U64 PRT_ClkCycle2Ms(U64 cycle)
{
    return DIV64(cycle, g_systemClock / OS_SYS_MS_PER_SECOND);
}

OS_SEC_L4_TEXT U64 PRT_ClkCycle2Us(U64 cycle)
{
    return DIV64(cycle, g_systemClock / OS_SYS_US_PER_SECOND);
}

OS_SEC_L4_TEXT U8 PRT_GetPrimaryCore(void)
{
    return g_primaryCoreId;
}