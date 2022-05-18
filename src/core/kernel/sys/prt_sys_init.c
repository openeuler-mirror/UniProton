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

OS_SEC_BSS U8 g_cpuType;

OS_SEC_L4_TEXT U8 OsGetCpuType(void)
{
    return g_cpuType;
}

OS_SEC_L4_TEXT U32 OsSysTimeHookReg(void)
{
    return OsSetSysTimeHook(PRT_ClkGetCycleCount64);
}

OS_SEC_L4_TEXT U32 OsSysRegister(struct SysModInfo *modInfo)
{
    U32 ret;

    if (modInfo->systemClock < OS_SYS_US_PER_SECOND) {
        return OS_ERRNO_SYS_CLOCK_INVALID;
    }

    ret = OsSysTimeHookReg();
    if (ret != OS_OK) {
        return ret;
    }

    UNI_FLAG = 0;
    g_systemClock = modInfo->systemClock;
    g_cpuType = (U8)modInfo->cpuType;
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
    if (OsHwiCheckMaxNum(modInfo->hwiMaxNum) == FALSE) {
        return OS_ERRNO_SYS_HWI_MAX_NUM_CONFIG_INVALID;
    }
    g_hwiMaxNumConfig = modInfo->hwiMaxNum;
#endif

    return ret;
}

OS_SEC_L4_TEXT U64 PRT_ClkCycle2Ms(U64 cycle)
{
    return DIV64(cycle, g_systemClock / OS_SYS_MS_PER_SECOND);
}

OS_SEC_L4_TEXT U64 PRT_ClkCycle2Us(U64 cycle)
{
    return DIV64(cycle, g_systemClock / OS_SYS_US_PER_SECOND);
}
