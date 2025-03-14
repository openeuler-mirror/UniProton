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
 * Create: 2024-03-07
 */

#include "armv8_pmu.h"
#include "perf/prt_perf_pmu.h"

#define STRINGIFY(x) #x

#define AARCH64_SYSREG_READ(reg)                                 \
    ({                                                           \
        U64 _val;                                                \
        __asm__ volatile("mrs %0," STRINGIFY(reg) : "=r"(_val)); \
        _val;                                                    \
    })

#define AARCH64_SYSREG_WRITE(reg, val)                             \
    ({                                                             \
        __asm__ volatile("msr " STRINGIFY(reg) ", %0" ::"r"(val)); \
        __asm__ volatile("isb" ::: "memory");                      \
    })

OS_SEC_BSS static HwPmu g_armv8Pmu;

static inline U32 Armv8PmcrRead(void)
{
    return (U32)AARCH64_SYSREG_READ(pmcr_el0);
}

static inline void Armv8PmcrWrite(U32 value)
{
    value &= ARMV8_PMCR_MASK;
    AARCH64_SYSREG_WRITE(pmcr_el0, value);
    return;
}

static inline U32 Armv8PmuOverflowed(U32 pmovsr)
{
    return pmovsr & ARMV8_OVERFLOWED_MASK;
}

static inline U32 Armv8PmuCntOverflowed(U32 pmovsr, U32 index)
{
    return pmovsr & ARMV8_CNT2BIT(ARMV8_IDX2CNT(index));
}

static inline U32 Armv8CntValid(U32 index)
{
    return index <= ARMV8_IDX_MAX_COUNTER;
}

static inline void Armv8PmuSelCnt(U32 index)
{
    U32 counter = ARMV8_IDX2CNT(index);
    AARCH64_SYSREG_WRITE(pmselr_el0, counter);
    return;
}

static inline void Armv8PmuSetCntPeriod(U32 index, U32 period)
{
    U64 value64;

    if (!Armv8CntValid(index)) {
        printf("CPU writing wrong counter %u\n", index);
    } else if (index == ARMV8_IDX_CYCLE_COUNTER) {
        value64 = ARMV8_PERIOD_MASK | period;
        AARCH64_SYSREG_WRITE(pmccntr_el0, value64);
    } else {
        Armv8PmuSelCnt(index);
        AARCH64_SYSREG_WRITE(pmxevcntr_el0, period);
    }

    return;
}

static inline void Armv8BindEvt2Cnt(U32 index, U32 value)
{
    Armv8PmuSelCnt(index);
    value &= ARMV8_EVTYPE_MASK;
    AARCH64_SYSREG_WRITE(pmxevtyper_el0, value);
    return;
}

static inline void Armv8EnableCnt(U32 index)
{
    U32 counter = ARMV8_IDX2CNT(index);
    AARCH64_SYSREG_WRITE(pmcntenset_el0, ARMV8_CNT2BIT(counter));
    return;
}

static inline void Armv8DisableCnt(U32 index)
{
    U32 counter = ARMV8_IDX2CNT(index);
    AARCH64_SYSREG_WRITE(pmcntenclr_el0, ARMV8_CNT2BIT(counter));
    return;
}

static inline void Armv8EnableCntInterrupt(U32 index)
{
    U32 counter = ARMV8_IDX2CNT(index);
    AARCH64_SYSREG_WRITE(pmintenset_el1, ARMV8_CNT2BIT(counter));
    return;
}

static inline void Armv8DisableCntInterrupt(U32 index)
{
    U32 counter = ARMV8_IDX2CNT(index);
    AARCH64_SYSREG_WRITE(pmintenclr_el1, ARMV8_CNT2BIT(counter));
    AARCH64_SYSREG_WRITE(pmovsclr_el0, ARMV8_CNT2BIT(counter));
    return;
}

static inline U32 Armv8PmuGetAndResetOverflowStatus(void)
{
    U32 flags;

    __asm__ volatile("mrs %0, pmovsclr_el0" : "=r" (flags));
    flags &= ARMV8_FLAG_MASK;
    __asm__ volatile("msr pmovsclr_el0, %0" :: "r" (flags));

    return flags;
}

static void Armv8EnableEvent(Event *event)
{
    U32 intSave;
    U32 cnt = event->counter;

    if (!Armv8CntValid(cnt)) {
        printf("CPU enabling wrong PMNC counter IRQ enable %u\n", cnt);
        return;
    }

    if (event->period == 0) {
        printf("event period value not valid, counter: %u\n", cnt);
        return;
    }

    // Enable counter and interrupt, and set the counter to count the event that we're interested in
    intSave = PRT_HwiLock();

    Armv8DisableCnt(cnt);
    Armv8BindEvt2Cnt(cnt, event->eventId);
    Armv8EnableCntInterrupt(cnt);
    Armv8EnableCnt(cnt);

    PRT_HwiRestore(intSave);

    return;
}

static void Armv8DisableEvent(Event *event)
{
    U32 intSave;
    U32 cnt = event->counter;

    if (!Armv8CntValid(cnt)) {
        printf("CPU enabling wrong PMNC counter IRQ enable %u\n", cnt);
        return;
    }

    // Disable counter and interrupt
    intSave = PRT_HwiLock();
    Armv8DisableCnt(cnt);
    Armv8DisableCntInterrupt(cnt);
    PRT_HwiRestore(intSave);
    return;
}

static inline void Armv8StartAllCnt(void)
{
    U32 value = Armv8PmcrRead() | ARMV8_PMCR_E;
    if (g_armv8Pmu.cntDivided) {
        value |= ARMV8_PMCR_D;
    } else {
        value &= ~ARMV8_PMCR_D;
    }

    Armv8PmcrWrite(value);
    PRT_HwiEnable(PRT_PMU_IRQ_NR);
    return;
}

static inline void Armv8StopAllCnt(void)
{
    Armv8PmcrWrite(Armv8PmcrRead() & ~ARMV8_PMCR_E);
    PRT_HwiDisable(PRT_PMU_IRQ_NR);
    return;
}

static inline void Armv8ResetAllCnt(void)
{
    U32 index;
    U32 reg;

    for (index = ARMV8_IDX_CYCLE_COUNTER; index < ARMV8_IDX_MAX_COUNTER; index++) {
        Armv8DisableCnt(index);
        Armv8DisableCntInterrupt(index);
    }

    /* Initialize & Reset PMNC: C and P bits and D bits */
    reg = ARMV8_PMCR_P | ARMV8_PMCR_C | (g_armv8Pmu.cntDivided ? ARMV8_PMCR_D : 0);
    Armv8PmcrWrite(reg);
    return;
}

static void Armv8SetEventPeriod(Event *event)
{
    if (event->period != 0) {
        Armv8PmuSetCntPeriod(event->counter, PERIOD_CALC(event->period));
    }
    return;
}

static inline uintptr_t Armv8ReadEventCnt(Event* event)
{
    U64 value = 0;
    U32 index = event->counter;

    if (!Armv8CntValid(index)) {
        printf("reading wrong counter %u\n", index);
    } else if (index == ARMV8_IDX_CYCLE_COUNTER) {
        value = AARCH64_SYSREG_READ(pmccntr_el0);
        value &= ARMV8_READ_MASK;
    } else {
        Armv8PmuSelCnt(index);
        value = AARCH64_SYSREG_READ(pmxevcntr_el0);
    }

    if (value < PERIOD_CALC(event->period)) {
        if (Armv8PmuCntOverflowed(Armv8PmuGetAndResetOverflowStatus(), event->counter)) {
            value += event->period;
        }
    } else {
        value -= PERIOD_CALC(event->period);
    }

    return value;
}

static const U32 g_armv8Map[] = {
    [PERF_COUNT_HW_CPU_CYCLES]          = ARMV8_PMUV3_PERF_HW_CLOCK_CYCLES,
    [PERF_COUNT_HW_INSTRUCTIONS]        = ARMV8_PMUV3_PERF_HW_INSTR_EXECUTED,
    [PERF_COUNT_HW_DCACHE_REFERENCES]   = ARMV8_PMUV3_PERF_HW_L1_DCACHE_ACCESS,
    [PERF_COUNT_HW_DCACHE_MISSES]       = ARMV8_PMUV3_PERF_HW_L1_DCACHE_REFILL,
    [PERF_COUNT_HW_ICACHE_REFERENCES]   = ARMV8_PMUV3_PERF_HW_L1_ICACHE_ACCESS,
    [PERF_COUNT_HW_ICACHE_MISSES]       = ARMV8_PMUV3_PERF_HW_L1_ICACHE_REFILL,
    [PERF_COUNT_HW_BRANCH_INSTRUCTIONS] = ARMV8_PMUV3_PERF_HW_PC_WRITE,
    [PERF_COUNT_HW_BRANCH_MISSES]       = ARMV8_PMUV3_PERF_HW_PC_BRANCH_MIS_PRED,
};

U32 Armv8PmuMapEvent(U32 eventType, U32 reverse)
{
    U32 i;

    if (!reverse) {
        /* common event to armv8 real event */
        if (eventType < ARRAY_SIZE(g_armv8Map)) {
            return g_armv8Map[eventType];
        }
        return PERF_HW_INVAILD_EVENT_TYPE;
    } else {
        /* armv8 real event to common event */
        for (i = 0; i < ARRAY_SIZE(g_armv8Map); i++) {
            if (g_armv8Map[i] == eventType) {
                return i;
            }
        }
        return PERF_HW_INVAILD_EVENT_TYPE;
    }
}

static void Armv8PmuIrqHandler(void)
{
    U32 index;
    U32 pmovsr;
    PerfRegs regs;
    PerfEvent *events = &(g_armv8Pmu.pmu.events);
    U32 eventNum = events->nr;

    pmovsr = Armv8PmuGetAndResetOverflowStatus();
    if (!Armv8PmuOverflowed(pmovsr)) {
        return;
    }

    (void)memset_s(&regs, sizeof(PerfRegs), 0, sizeof(PerfRegs));
    OsPerfFetchIrqRegs(&regs);

    Armv8StopAllCnt();
    for (index = 0; index < eventNum; index++) {
        Event *event = &(events->per[index]);
        // We have a single interrupt for all counters. Check that each counter has overflowed before we process it.
        if (!Armv8PmuCntOverflowed(pmovsr, event->counter) || (event->period == 0)) {
            continue;
        }

        Armv8PmuSetCntPeriod(event->counter, PERIOD_CALC(event->period));

        OsPerfUpdateEventCount(event, event->period);
        OsPerfHandleOverFlow(event, &regs);
    }
    Armv8StartAllCnt();

    return;
}

U32 OsGetPmuMaxCounter(void)
{
    return ARMV8_IDX_MAX_COUNTER;
}

U32 OsGetPmuCycleCounter(void)
{
    return ARMV8_IDX_CYCLE_COUNTER;
}

U32 OsGetPmuCounter0(void)
{
    return ARMV8_IDX_COUNTER0;
}

static HwPmu g_armv8Pmu = {
    .canDivided = TRUE,
    .enable     = Armv8EnableEvent,
    .disable    = Armv8DisableEvent,
    .start      = Armv8StartAllCnt,
    .stop       = Armv8StopAllCnt,
    .clear      = Armv8ResetAllCnt,
    .setPeriod  = Armv8SetEventPeriod,
    .readCnt    = Armv8ReadEventCnt,
    .mapEvent   = Armv8PmuMapEvent,
};

U32 OsHwPmuInit(void)
{
    U32 ret;

    ret = PRT_HwiSetAttr(PRT_PMU_IRQ_NR, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        printf("pmu %d irq set attri failed, ret=0x%x\n", PRT_PMU_IRQ_NR, ret);
        return ret;
    }

    ret = PRT_HwiCreate(PRT_PMU_IRQ_NR, Armv8PmuIrqHandler, 0);
    if (ret != OS_OK) {
        printf("pmu %d irq handler register failed, ret=0x%x\n", PRT_PMU_IRQ_NR, ret);
        return ret;
    }

    ret = OsPerfHwInit(&g_armv8Pmu);
    return ret;
}