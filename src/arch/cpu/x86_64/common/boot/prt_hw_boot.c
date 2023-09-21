/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-18
 * Description: 硬件相关的通用处理。
 */
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_attr_external.h"
#include "prt_tick_external.h"
#include "../hwi/prt_lapic.h"

/* 系统当前运行的时间，单位cycle */
OS_SEC_BSS U64 g_cycleNow;
/* 系统当前运行的时间，时间是g_cyclePerTick的整数倍 */
// OS_SEC_BSS U64 g_cycleByTickNow;

extern volatile U64 g_uniTicks;

OS_SEC_L2_TEXT void OsCycleUpdate(void)
{
    U64 hwCycleFirst;
    U64 hwCycleSecond;

    // 假设 A 和 B 之间如果发生tick中断, 则 hwCycleFirst < hwCycleSecond
    U64 tick1 = g_uniTicks;
    OsReadMsr(X2APIC_TCCR, &hwCycleFirst); // A
    U64 tick2 = g_uniTicks;
    OsReadMsr(X2APIC_TCCR, &hwCycleSecond); // B

    /* 发生tick反转，需要手动补偿一个周期 */
    if (hwCycleFirst < hwCycleSecond) {
        tick2 = tick1 + 1;
    }
    g_cycleNow = (tick2 * OsGetCyclePerTick() + (OsGetCyclePerTick() - hwCycleSecond));
}

OS_SEC_L2_TEXT U64 PRT_ClkGetCycleCount64(void)
{
    U64 cycle;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    OsCycleUpdate();
    cycle = g_cycleNow;
    PRT_HwiRestore(intSave);
    return cycle;
}
