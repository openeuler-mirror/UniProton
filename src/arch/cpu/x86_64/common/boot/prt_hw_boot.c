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
#include "prt_hwi.h"
#include "../hwi/prt_lapic.h"

OS_SEC_BSS U32 g_tscEAX = 1;
OS_SEC_BSS U32 g_tscEBX = 1;

OS_SEC_BSS U64 g_cycleNow;
OS_SEC_BSS U8 g_useTsc = 0;
extern volatile U64 g_uniTicks;

#define BIT(nr) (1UL << (nr))

bool OsGetTscRatio(void)
{
    U32 a, b, c, d;
    a = 0x15;
    __asm__ volatile("cpuid"
        : "=a" (a), "=b" (b), "=c" (c), "=d" (d)
        : "a" (a), "b" (b),"c" (c), "d" (d)
        );
    if (b != 0) {
        g_tscEAX = a;
        g_tscEBX = b;
        return true;
        
    }
    return false;
}

/* support rdtsc instruction */
bool OsIsTscSupport(void)
{
    U32 a, b, c, d;
    a = 0x1;
    __asm__ volatile("cpuid"
        : "=a" (a), "=b" (b), "=c" (c), "=d" (d)
        : "a" (a), "b" (b),"c" (c), "d" (d)
        );
    if (d & BIT(4)) {
        return true;
    }
    return false;
}

/* invariant tsc */
bool OsIsTscInvariant(void)
{
    U32 a, b, c, d;
    a = 0x80000007;
    __asm__ volatile("cpuid"
        : "=a" (a), "=b" (b), "=c" (c), "=d" (d)
        : "a" (a), "b" (b),"c" (c), "d" (d)
        );
    if (d & BIT(8)) {
        return true;
    }
    return false;
}

OS_SEC_L2_TEXT U64 OsLapicCycleUpdate(void) {
    U64 hwCycle;
    U64 hwCycleSecond;
    U64 lvtTimerReg;
    U64 tick = g_uniTicks;

    OsReadMsr(X2APIC_TCCR, &hwCycle);
    OsReadMsr(LAPIC_LVT_TIMER, &lvtTimerReg);
    OsReadMsr(X2APIC_TCCR, &hwCycleSecond);

    // 如果发生中断pending，使用确定发生在pending后的cycle值
    if (lvtTimerReg & BIT(12)) {
        hwCycle = hwCycleSecond;
        tick++;
    }

    return (tick * OsGetCyclePerTick() + (OsGetCyclePerTick() - hwCycle));
}

void OsCycleInit(void)
{
    if (OsIsTscSupport() && OsGetTscRatio() && OsIsTscInvariant()) {
        g_useTsc = 1;
    } else {
        g_useTsc = 0;
    }
}

OS_SEC_ALW_INLINE INLINE U64 OsReadTsc(void)
{
    U32 eax;
    U32 edx;
    __asm__ volatile("rdtsc" : "=a"(eax), "=d"(edx));
    return ((U64)edx << 32) | eax;
}

/* tsc freq = core crystal freq * CPUID.15H.EBX / CPUID.15H.EAX */
/* LAPIC freq is same as core crystal freq */
OS_SEC_L2_TEXT U64 OsClkGetTscCycleCount64(void)
{
    // change tsc cycle to core crystal cycle
    return OsReadTsc() * g_tscEAX / g_tscEBX;
}

OS_SEC_L2_TEXT U64 OsClkGetLapicCycleCount64(void)
{
    U64 cycle;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    cycle = OsLapicCycleUpdate();
    PRT_HwiRestore(intSave);
    return cycle;
}

OS_SEC_L2_TEXT U64 PRT_ClkGetCycleCount64(void)
{
    if (g_useTsc) {
        return OsClkGetTscCycleCount64();
    }
    return OsClkGetLapicCycleCount64();
}
