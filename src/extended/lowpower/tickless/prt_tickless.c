/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Description: Tickless adapter. Preserves the LiteOs OsTickless* flow on top of
 *              the native UniProton tickless kernel interface
 *              (PRT_TickLessCountGet / PRT_TickCountUpdate). A board may register
 *              TicklessOps to drive the actual wakeup-timer reprogramming; when no
 *              ops are registered, a default armv8 generic-timer implementation is
 *              used so that the feature works out of the box on armv8 boards.
 */
#include "prt_tickless.h"
#include "prt_tick.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_sys_external.h"

#if defined(OS_OPTION_TICKLESS)

OS_SEC_BSS static volatile bool g_ticklessFlag = false;
OS_SEC_BSS static volatile bool g_tickIrqFlag = false;
OS_SEC_BSS static volatile U32 g_sleepTicks = 0;

OS_SEC_BSS static const struct TicklessOps *g_ticklessOps = NULL;
/* Board tick IRQ number (set via PRT_TicklessSetTickIrqNum); 0xFFFFFFFF = unset. */
OS_SEC_BSS static U32 g_tickIrqNum = 0xFFFFFFFFU;

static inline U32 OsGenericTimerFreqGet(void)
{
#if defined(OS_ARCH_ARMV8)
    U64 freq = 0;
    OS_EMBED_ASM("mrs %0, CNTFRQ_EL0" : "=r"(freq) : : "memory", "cc");
    return (U32)freq;
#else
    return 0;
#endif
}

static inline U32 OsCyclesPerTickGet(void)
{
    U32 freq = OsGenericTimerFreqGet();
    U32 ticksPerSec = OsSysGetTickPerSecond();
    if ((freq == 0) || (ticksPerSec == 0)) {
        return 0;
    }
    return freq / ticksPerSec;
}

static inline U32 OsTickTimerCyclesGet(void)
{
    U32 cycles = 0;
#if defined(OS_ARCH_ARMV8)
    OS_EMBED_ASM("mrs %0, CNTP_TVAL_EL0" : "=r"(cycles) : : "memory", "cc");
#else
    cycles = 0;
#endif
    return cycles;
}

static inline void OsTickTimerReload(U32 cycles)
{
#if defined(OS_ARCH_ARMV8)
    U32 ctl = 0;
    OS_EMBED_ASM("msr CNTP_CTL_EL0, %0" : : "r"(ctl) : "memory");
    PRT_ISB();
    OS_EMBED_ASM("msr CNTP_TVAL_EL0, %0" : : "r"(cycles) : "memory", "cc");
    ctl = 1;
    OS_EMBED_ASM("msr CNTP_CTL_EL0, %0" : : "r"(ctl) : "memory");
#endif
}

static inline void OsTickTimerReloadDefault(U32 sleepTicks)
{
    U32 cyclesPerTick = OsCyclesPerTickGet();
    if ((cyclesPerTick == 0) || (sleepTicks == 0)) {
        return;
    }
    OsTickTimerReload(sleepTicks * cyclesPerTick);
}

static inline U32 OsTickTimerCyclesLeftGetDefault(void)
{
    return OsTickTimerCyclesGet();
}

static inline void OsTickTimerRestoreDefault(void)
{
    OsTickTimerReload(OsCyclesPerTickGet());
}

void PRT_TicklessOpsReg(const struct TicklessOps *ops)
{
    g_ticklessOps = ops;
}

void PRT_TicklessSetTickIrqNum(U32 irqNum)
{
    g_tickIrqNum = irqNum;
}

void PRT_TicklessEnable(void)
{
    g_ticklessFlag = true;
}

void PRT_TicklessDisable(void)
{
    g_ticklessFlag = false;
}

bool OsTicklessFlagGet(void)
{
    return g_ticklessFlag;
}

bool OsTickIrqFlagGet(void)
{
    return g_tickIrqFlag;
}

void OsTickIrqFlagSet(bool tickIrqFlag)
{
    g_tickIrqFlag = tickIrqFlag;
}

U32 OsTicklessSleepTickGet(void)
{
    return g_sleepTicks;
}

void OsTicklessSleepTickSet(U32 sleeptick)
{
    g_sleepTicks = sleeptick;
}

/* LiteOs OsSleepTicksGet: mapped to the native UniProton tickless query. */
U32 OsSleepTicksGet(void)
{
    U32 minTicks = 0;
    U32 coreId = 0;

    /* PRT_TickLessCountGet sets *minTicks = OS_MAX_U32 and returns FAIL when
     * every core's nearest event is FOREVER (idle). LiteOs's OsSleepTicksGet
     * returns a large value in that case so idle takes managed light sleep;
     * returning 0 here made getSleepTime() always 0 -> work-mode flood on every
     * idle iteration. Return minTicks (== OS_MAX_U32 when idle). */
    (void)PRT_TickLessCountGet(&minTicks, &coreId);
    return minTicks;
}

/* LiteOs OsSysTimeUpdate: mapped to the native UniProton tick compensation. */
void OsSysTimeUpdate(U32 sleepTicks)
{
    if (sleepTicks == 0) {
        return;
    }
    (void)PRT_TickCountUpdate(OS_TYPE_LIGHT_SLEEP, (S32)sleepTicks);
}

/* LiteOs OsTicklessUpdate: compensate time after wakeup from interrupt.
 * UniProton's PRT_TickCountUpdate(N) does g_uniTicks += N (full N), whereas
 * LiteOs OsSysTimeUpdate does += N-1. So the compensation passed here is the
 * "extra" beyond the +1 that OsTickDispatcher adds on a tick IRQ. */
void OsTicklessUpdate(U32 irqNum)
{
    U32 sleepTicks;
    uintptr_t intSave;

    intSave = OsIntLock();
    sleepTicks = OsTicklessSleepTickGet();
    if (sleepTicks == 0) {
        OsIntRestore(intSave);
        return;
    }

    if (g_ticklessOps != NULL && g_ticklessOps->update != NULL) {
        (void)g_ticklessOps->update(irqNum);
    } else if (irqNum == g_tickIrqNum) {
        /* Tick one-shot fully elapsed: physical time advanced by `sleepTicks`.
         * UniProton splits tick accounting into two stages: this wakeup hook
         * (OsTicklessUpdate, controllable) + the deferred-tick tail
         * (while(tickNoRespondCnt>0) OsTickDispatcher, which does g_uniTicks += 1
         * once per PRT_TickISR). PRT_TickISR ran for this tick IRQ, so the tail
         * WILL add +1. The two stages must sum to sleepTicks, hence compensate
         * sleepTicks-1 here. (LiteOs OsSysTimeUpdate(N) internally does += N-1,
         * so it passes sleepTicks and gets += sleepTicks-1 -- equivalent.) */
        OsSysTimeUpdate(sleepTicks - 1);
    } else {
        /* Early wake by a non-tick IRQ: PRT_TickISR did NOT run, so the deferred
         * tail adds 0 and there is no +1. Compensate the elapsed cycles only and
         * re-arm the timer for the remainder. */
        U32 cyclesPerTick = OsCyclesPerTickGet();
        if (cyclesPerTick != 0) {
            U32 cycles = OsTickTimerCyclesLeftGetDefault();
            U32 elapsed = (sleepTicks * cyclesPerTick) - cycles;
            U32 ticks = elapsed / cyclesPerTick;
            if (ticks < sleepTicks) {
                OsSysTimeUpdate(ticks);
                OsTickTimerReload(cyclesPerTick - (elapsed % cyclesPerTick));
            } else {
                OsSysTimeUpdate(sleepTicks - 1);
            }
        } else {
            OsSysTimeUpdate(sleepTicks - 1);
        }
    }

    OsTicklessSleepTickSet(0);
    OsIntRestore(intSave);
}

/* LiteOs OsTicklessStart: reprogram the wakeup timer for the sleep duration. */
void OsTicklessStart(void)
{
    U32 cyclesPerTick;
    U32 maxTicks;
    U32 sleepTicks;
    U32 cycles;
    U32 cyclesPre;
    U32 cyclesCur;
    U32 cycleCompensate;
    uintptr_t intSave;

    cyclesPerTick = OsCyclesPerTickGet();
    if (cyclesPerTick == 0) {
        return;
    }
    maxTicks = U32_MAX / cyclesPerTick;

    intSave = OsIntLock();
    sleepTicks = OsSleepTicksGet();
    cyclesPre = OsTickTimerCyclesLeftGetDefault();

    if (sleepTicks > 1) {
        if (sleepTicks >= maxTicks) {
            sleepTicks = maxTicks;
        }
        cycles = sleepTicks * cyclesPerTick;
        cyclesCur = OsTickTimerCyclesLeftGetDefault();
        if (cyclesPre > cyclesCur) {
            cycleCompensate = cyclesPre - cyclesCur;
        } else {
            cycleCompensate = (cyclesPerTick + cyclesPre) - cyclesCur;
        }
        if (cycles > cycleCompensate) {
            cycles -= cycleCompensate;
        } else {
            cycles = cyclesPerTick;
        }
        if (g_ticklessOps != NULL && g_ticklessOps->start != NULL) {
            g_ticklessOps->start(sleepTicks);
        } else {
            OsTickTimerReload(cycles);
        }
        OsTicklessSleepTickSet(sleepTicks);
        OsIntRestore(intSave);
        return;
    }
    OsIntRestore(intSave);
}

void OsTicklessOpen(void)
{
    if (OsTickIrqFlagGet()) {
        OsTickIrqFlagSet(false);
        OsTicklessStart();
    }
}

void OsTicklessDone(void)
{
    OsTicklessSleepTickSet(0);
}

U32 OsTicklessGetSleepTicks(void)
{
    return OsSleepTicksGet();
}

#endif /* OS_OPTION_TICKLESS */
