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
#include "prt_attr_external.h"
#include "prt_sys_external.h"

/* 用户注册的获取系统时间钩子 */
OS_SEC_BSS SysTimeFunc g_sysTimeHook;

OS_SEC_L2_TEXT U32 OsSetSysTimeHook(SysTimeFunc hook)
{
    /* 钩子不支持重复注册 */
    if (g_sysTimeHook != NULL) {
        return OS_ERRNO_SYS_TIME_HOOK_REGISTER_REPEATED;
    }
    g_sysTimeHook = hook;
    return OS_OK;
}

/*
 * 描述：当前系统时间
 */
OS_SEC_L2_TEXT U64 OsCurCycleGet64(void)
{
    if (g_sysTimeHook != NULL) {
        return g_sysTimeHook();
    } else {
        return 0;
    }
}

/*
 * 描述：获取当前的tick计数
 */
OS_SEC_L2_TEXT U64 PRT_TickGetCount(void)
{
    return g_uniTicks;
}

/*
 * 描述：根据输入的周期数进行延时
 */
OS_SEC_ALW_INLINE INLINE void OsTimerDelayCount(U64 cycles)
{
    U64 cur;
    U64 end;

    cur = OsCurCycleGet64();
    end = cur + cycles;

    if (cur < end) {
        while (cur < end) {
            cur = OsCurCycleGet64();
        }
    } else {
        /* 考虑Cycle计数反转的问题，虽然64位的计数需要584年才能反转，但是考虑软件的严密性，需要做反转保护 */
        /* 当前值大于结束值的场景 */
        while (cur > end) {
            cur = OsCurCycleGet64();
        }
        /* 正过来之后追赶的场景 */
        while (cur < end) {
            cur = OsCurCycleGet64();
        }
    }
}

/*
 * 描述：时间延迟毫秒
 */
OS_SEC_L2_TEXT void PRT_ClkDelayMs(U32 delay)
{
    U64 cycles;

    /* 将毫秒转化为Cycle计数，例如1m为1M个cycle */
    cycles = OS_MS2CYCLE(delay, g_systemClock);

    OsTimerDelayCount(cycles);
}

/*
 * 描述：时间延迟微秒
 */
OS_SEC_L4_TEXT void PRT_ClkDelayUs(U32 delay)
{
    U64 cycles;

    cycles = OS_US2CYCLE(delay, g_systemClock);

    OsTimerDelayCount(cycles);
}
