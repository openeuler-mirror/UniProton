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
 * Description: Low-power framework. Direct port of LiteOs los_lowpower.c;
 *              kernel interfaces are replaced with UniProton PRT_ equivalents
 *              (idle hook: PRT_IdleAddHook, interrupt lock: OsIntLock, wfi: asm).
 */
#include "prt_lowpower.h"
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_idle.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "prt_cpu_external.h"
#include "prt_task_external.h"
#if defined(OS_OPTION_TICKLESS)
#include "prt_tickless.h"
#endif

extern U32 PRT_Printf(const char *format, ...);

#define CALL_PMOPS_FUNC_VOID(func, ...) CALL_FUNC_NO_RETURN_FROM_PTR(g_pmOps, func, __VA_ARGS__)
#define CALL_PMOPS_FUNC_RET(func, ret, ...) CALL_FUNC_WITH_RETURN_FROM_PTR(g_pmOps, ret, func, __VA_ARGS__)

#define CALL_FUNC_NO_RETURN_FROM_PTR(ops, func, ...) \
    do {                                             \
        if (((ops) != NULL) && ((ops)->func != NULL)) { \
            (ops)->func(__VA_ARGS__);                \
        }                                            \
    } while (0)

#define CALL_FUNC_WITH_RETURN_FROM_PTR(ops, ret, func, ...) \
    do {                                                    \
        if (((ops) != NULL) && ((ops)->func != NULL)) {     \
            (ret) = (ops)->func(__VA_ARGS__);               \
        }                                                   \
    } while (0)

static inline void OsAsmWfi(void)
{
#if defined(OS_ARCH_ARMV8) || defined(OS_ARCH_ARMV7) || defined(OS_ARCH_AARCH32)
    OS_EMBED_ASM("wfi" : : : "memory");
#else
    /* Non-ARM ports: busy-no-op, replaced by the platform's core sleep. */
#endif
}

/* CPU idle sleep for the kernel idle loop's g_taskCoreSleep slot. The idle task
 * (OsTskIdleBgd) calls g_taskCoreSleep after the idle hooks; the kernel leaves
 * it NULL, so wiring it to wfi keeps the CPU sleep in coreSleep -- where it
 * belongs -- and lets idle hooks do decision work only instead of also doing
 * the wfi. A BSP may set its own (deeper) core sleep before PRT_LowpowerInit. */
static void OsLowpowerCoreSleep(void)
{
    OsAsmWfi();
}

OS_SEC_DATA static const struct PowerMgrOps *g_pmOps = NULL;
OS_SEC_DATA static IntWakeupHookFn g_intWakeupHook = NULL;

void PRT_LowpowerHookReg(LowPowerHookFn hook)
{
    (void)PRT_IdleAddHook(hook);
}

void PRT_IntWakeupHookReg(IntWakeupHookFn hook)
{
    g_intWakeupHook = hook;
}

void OsLowpowerIntWakeupHookCall(U32 hwiNum)
{
    if (g_intWakeupHook != NULL) {
        g_intWakeupHook(hwiNum);
    }
}

void PRT_LowpowerInit(const struct PowerMgrOps *pmOps)
{
    if (pmOps == NULL) {
        PRT_Printf("\r\n [PM] PowerMgrOps must be non-null.\n");
        return;
    } else if (pmOps->process == NULL) {
        PRT_Printf("\r\n [PM] PRT_LowpowerInit must be non-null.\n");
        return;
    }

#if defined(OS_OPTION_POWERMGR)
    if (g_pmOps != NULL) {
        PRT_Printf("\r\n [PM] Reassignment of PowerMgrOps is forbidden.\n");
        return;
    }
#endif
    g_pmOps = pmOps;

    PRT_LowpowerHookReg(OsPowerMgrProcess);
    PRT_IntWakeupHookReg(OsPowerMgrWakeUpFromInterrupt);

#if defined(OS_OPTION_TICKLESS)
    /* Enable tickless at low-power init (LiteOs enables it in los_init). Once
     * set, each tick IRQ sets g_tickIrqFlag so the idle hook's OsTicklessOpen
     * reprograms the wake timer to one-shot for the next sleep budget. */
    PRT_TicklessEnable();
#endif

    /* Wire the idle task's CPU-sleep slot (left NULL by the kernel) to wfi so
     * the idle task actually sleeps instead of busy-looping. Only wire the
     * default if a BSP has not already set a (deeper) core sleep. */
    if (g_taskCoreSleep == NULL) {
        g_taskCoreSleep = OsLowpowerCoreSleep;
    }
}

void OsPowerMgrProcess(void)
{
#if defined(OS_OPTION_POWERMGR)
    CALL_PMOPS_FUNC_VOID(process);
#else
    if (g_pmOps == NULL) {
#if defined(OS_OPTION_TICKLESS)
        OsTicklessOpen();
        OsAsmWfi();
#endif
    } else {
        CALL_PMOPS_FUNC_VOID(process);
    }
#endif
}

void OsPowerMgrWakeUpFromInterrupt(U32 intNum)
{
#if defined(OS_OPTION_POWERMGR)
    CALL_PMOPS_FUNC_VOID(resumeFromInterrupt, intNum);
#else
    if (g_pmOps == NULL) {
#if defined(OS_OPTION_TICKLESS)
        OsTicklessUpdate(intNum);
#endif
    } else {
        CALL_PMOPS_FUNC_VOID(resumeFromInterrupt, intNum);
    }
#endif
}

void OsPowerMgrWakeupFromReset(void)
{
    CALL_PMOPS_FUNC_VOID(wakeupFromReset);
}

void PRT_PowerMgrChangeFreq(enum PRT_FreqMode freq)
{
    CALL_PMOPS_FUNC_VOID(changeFreq, freq);
}

void PRT_PowerMgrDeepSleepVoteBegin(void)
{
    CALL_PMOPS_FUNC_VOID(deepSleepVoteBegin);
}

void PRT_PowerMgrDeepSleepVoteEnd(void)
{
    CALL_PMOPS_FUNC_VOID(deepSleepVoteEnd);
}

void PRT_PowerMgrSleepDelay(U32 tick)
{
    CALL_PMOPS_FUNC_VOID(deepSleepVoteDelay, tick);
}

void PRT_PowerMgrRegisterExtVoter(LowpowerExternalVoterHandle callback)
{
    CALL_PMOPS_FUNC_VOID(registerExternalVoter, callback);
}

U32 PRT_PowerMgrGetSleepMode(void)
{
    U32 ret = 0;
    CALL_PMOPS_FUNC_RET(getSleepMode, ret);
    return ret;
}

U32 PRT_PowerMgrGetDeepSleepVoteCount(void)
{
    U32 ret = 0;
    CALL_PMOPS_FUNC_RET(getDeepSleepVoteCount, ret);
    return ret;
}
