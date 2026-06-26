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
 * Description: Power manager implementation. Direct port of LiteOs
 *              los_lowpower_impl.c. The flow is unchanged; only kernel interface
 *              dependencies are replaced with UniProton PRT_ equivalents
 *              (atomic/spinlock/int-lock/task-lock/tickless/runstop hooks).
 *              Deep-sleep specifics are gated by OS_OPTION_DEEPSLEEP (inactive on
 *              sd3403), so the active path is light-sleep + runstop.
 */
#include "prt_lowpower_impl.h"
#include "prt_lowpower.h"
#include "prt_lowpower_context.h"
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_atomic.h"
#include "prt_cpu_external.h"
#include "prt_idle.h"
#include "prt_tick.h"
#if defined(OS_OPTION_TICKLESS)
#include "prt_tickless.h"
#endif
#if defined(OS_OPTION_RUNSTOP)
#include "prt_runstop.h"
#endif

extern U32 PRT_Printf(const char *format, ...);

/* Runstop imaging state, mirroring LiteOs OsWowSysDoneFlagGet values. */
#define OS_NO_STORE_SYSTEM 0
#define OS_STORE_SYSTEM   1

/* Atomic helpers mapping LiteOs LOS_Atomic* onto UniProton primitives. */
#define PM_ATOMIC_SET(a, v)        (*(volatile S32 *)(a) = (S32)(v))
#define PM_ATOMIC_READ(a)          (*(volatile S32 *)(a))
#define PM_ATOMIC_INC(a)           (void)PRT_AtomicAdd32((S32 *)(a), 1)
#define PM_ATOMIC_DEC(a)           (void)PRT_AtomicAdd32((S32 *)(a), -1)
#define PM_ATOMIC_ADD(a, n)        (void)PRT_AtomicAdd32((S32 *)(a), (S32)(n))
#define PM_ATOMIC_SUB(a, n)        (void)PRT_AtomicAdd32((S32 *)(a), (S32)(-(n)))
/* PM_ATOMIC_CMPXCHG returns true on CAS success (*ptr==old, then set new).
 * NOTE: UniProton PRT_AtomicCompareAndStore32 returns true==success, the OPPOSITE
 * of LiteOs LOS_AtomicCmpXchg32bits (true==failure). Callers copied from LiteOs
 * must therefore invert the success/fail branch (see OsChangeFreq). */
#define PM_ATOMIC_CMPXCHG(a, n, o) (PRT_AtomicCompareAndStore32((U32 *)(a), (U32)(o), (U32)(n)) != 0)

#define LOS_ASSERT(x)
#define PRINT_ERR(...)   PRT_Printf("\r\n [PM] " __VA_ARGS__)
#define PRINT_DEBUG(...) PRT_Printf("\r\n [PM] " __VA_ARGS__)
#define PRINT_WARN(...)  PRT_Printf("\r\n [PM] " __VA_ARGS__)

static inline bool FreqHigher(enum PRT_FreqMode f1, enum PRT_FreqMode f2)
{
    return (U32)f1 < (U32)f2;
}

static inline void OsAsmWfi(void)
{
#if defined(OS_ARCH_ARMV8) || defined(OS_ARCH_ARMV7) || defined(OS_ARCH_AARCH32)
    OS_EMBED_ASM("wfi" : : : "memory");
#endif
}

#if defined(OS_OPTION_RUNSTOP) || defined(OS_OPTION_DEEPSLEEP)
/* Non-zero when the system is resuming from a saved image (shared with runstop). */
OS_SEC_DATA volatile U32 g_lowpowerOtherCoreResume = 0;
#endif

#ifndef OFFSET_OF_FIELD
#define OFFSET_OF_FIELD(type, field) ((uintptr_t) & ((type *)0)->field)
#endif

#define RUNOPS_CALL_FUNC_VOID(func, ...) CALL_FUNC_NO_RETURN(g_pmRunOps, func, __VA_ARGS__)
#define CALL_RUN_OPS_FUNC_NO_RETURN(func) CallVoidFunction(&g_pmRunOps, OFFSET_OF_FIELD(struct PowerMgrRunOps, func))
#if defined(OS_OPTION_DEEPSLEEP)
#define DEEPOPS_CALL_FUNC_VOID(func, ...) CALL_FUNC_NO_RETURN(g_deepSleepOps, func, __VA_ARGS__)
#else
#define DEEPOPS_CALL_FUNC_VOID(func, ...)
#endif

#define CALL_FUNC_NO_RETURN(ops, func, ...) \
    do {                                    \
        if ((ops).func != NULL) {           \
            (ops).func(__VA_ARGS__);        \
        }                                   \
    } while (0)

#define CALL_FUNC_WITH_RETURN(ops, ret, func, ...) \
    do {                                           \
        if ((ops).func != NULL) {                  \
            (ret) = (ops).func(__VA_ARGS__);       \
        }                                          \
    } while (0)

static inline void CallVoidFunction(void *obj, uintptr_t offset)
{
    if (obj != NULL) {
        void (*func)(void) = (void (*)(void))((uintptr_t)obj + offset);
        if (func != NULL) {
            func();
        }
    }
}

#ifdef LOSCFG_LOWPOWER_TRACE_DEBUG
#define TRACE_FUNC_CALL() PRT_Printf("trace: %s, %s, %d\n", __FILE__, __FUNCTION__, __LINE__)
#else
#define TRACE_FUNC_CALL()
#endif

#define MAX_SLEEP_TIME      10000
#define MIN_DEEP_SLEEP_TIME 1000
#define MIN_SLEEP_TIME      100

typedef volatile S32 Atomic;

typedef struct {
    struct PowerMgrDeepSleepOps *deepSleepOps;
    LowpowerExternalVoterHandle exVoterHandle;
    Atomic sleepVoteCount;
    U32 minSleepTicks;
    U32 maxSleepCount;
    U32 minDeepSleepTicks;
    U32 sleepTime[OS_MAX_CORE_NUM];
    Atomic deepSleepCores;
    Atomic resumeSleepCores;
    Atomic freq;
    Atomic freqPending;
    Atomic deepSleepDelay;
    Atomic freeLock;
    struct PrtSpinLock lock;
    U8 sleepMode[OS_MAX_CORE_NUM];
} PowerMgr;

enum { LOCK_OFF = 0, LOCK_ON };

OS_SEC_DATA static PowerMgr g_pmMgr = {
    .deepSleepOps = NULL,
    .exVoterHandle = NULL,
    .sleepVoteCount = 0,
    .maxSleepCount = MAX_SLEEP_TIME,
    .minSleepTicks = MIN_SLEEP_TIME,
    .minDeepSleepTicks = MIN_DEEP_SLEEP_TIME,
    .sleepTime = {0},
    .deepSleepCores = 0,
    .resumeSleepCores = 0,
    .freq = 1,
    .freqPending = 0,
    .deepSleepDelay = 0,
    .freeLock = 0,
    .sleepMode = {0},
};

static void OsChangeFreq(void);
static void OsLightSleepDefault(U32 sleepTick);
static void OsSetWakeUpTimerDefault(U32 sleepTick);
static U32 OsWithrawWakeUpTimerDefault(void);
static U32 OsGetSleepTimeDefault(void);
static U32 OsSelectSleepModeDefault(U32 sleepTicks);
static void OsChangeFreqDefault(U8 freq);
static void OsEnterDeepSleepDefault(void);
static U32 OsPreConfigDefault(void);
static void OsPostConfigDefault(void);

static struct PowerMgrRunOps g_pmRunOps = {
    .changeFreq = OsChangeFreqDefault,
    .enterLightSleep = OsLightSleepDefault,
    .enterDeepSleep = OsEnterDeepSleepDefault,
    .setWakeUpTimer = OsSetWakeUpTimerDefault,
    .withdrawWakeUpTimer = OsWithrawWakeUpTimerDefault,
    .getSleepTime = OsGetSleepTimeDefault,
    .selectSleepMode = OsSelectSleepModeDefault,
    .preConfig = OsPreConfigDefault,
    .postConfig = OsPostConfigDefault,
    .contextSave = NULL,
    .contextRestore = NULL,
    .getDeepSleepVoteCount = NULL,
    .getSleepMode = NULL,
    .setSleepMode = NULL,
};

static void OsLightSleepDefault(U32 sleepTick)
{
    TRACE_FUNC_CALL();
    (void)sleepTick;
    OsAsmWfi();
}

static void OsSetWakeUpTimerDefault(U32 sleepTick)
{
    TRACE_FUNC_CALL();
    (void)sleepTick;
}

static U32 OsWithrawWakeUpTimerDefault(void)
{
    TRACE_FUNC_CALL();
    return 0;
}

static U32 OsGetSleepTimeDefault(void)
{
#if defined(OS_OPTION_TICKLESS)
    return OsSleepTicksGet();
#else
    return 0;
#endif
}

static U32 OsSelectSleepModeDefault(U32 sleepTicks)
{
    if (sleepTicks < g_pmMgr.minSleepTicks) {
        return PRT_INTERMIT_NONE;
    }

#if defined(OS_OPTION_DEEPSLEEP)
    if (g_pmMgr.deepSleepOps != NULL && sleepTicks >= g_pmMgr.minDeepSleepTicks &&
        PRT_PowerMgrGetDeepSleepVoteCount() == 0) {
        return PRT_INTERMIT_DEEP_SLEEP;
    }
#endif
    return PRT_INTERMIT_LIGHT_SLEEP;
}

static void OsChangeFreqDefault(U8 freq)
{
    (void)freq;
    TRACE_FUNC_CALL();
}

static void OsEnterDeepSleepDefault(void)
{
    TRACE_FUNC_CALL();
    OsAsmWfi();
}

static U32 OsPreConfigDefault(void)
{
    TRACE_FUNC_CALL();
    return 1;
}

static void OsPostConfigDefault(void)
{
    TRACE_FUNC_CALL();
}

#if defined(OS_OPTION_DEEPSLEEP)

static bool OsCouldDeepSleepDefault(void)
{
    TRACE_FUNC_CALL();
    return true;
}

static bool OsSuspendPreConfigDefault(void)
{
    TRACE_FUNC_CALL();
    return true;
}

static void OsSuspendDeviceDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsRollBackDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsResumeDeviceDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsResumePostConfigDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsSystemWakeupDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsResumeCallBackDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsOtherCoreResumeDefault(void)
{
    TRACE_FUNC_CALL();
}

static void OsDeepSleepResume(void);

static struct PowerMgrDeepSleepOps g_deepSleepOps = {
    .couldDeepSleep = OsCouldDeepSleepDefault,
    .systemWakeup = OsSystemWakeupDefault,
    .suspendPreConfig = OsSuspendPreConfigDefault,
    .suspendDevice = OsSuspendDeviceDefault,
    .rollback = OsRollBackDefault,
    .resumeDevice = OsResumeDeviceDefault,
    .resumePostConfig = OsResumePostConfigDefault,
    .resumeCallBack = OsResumeCallBackDefault,
    .otherCoreResume = OsOtherCoreResumeDefault,
    .resumeFromReset = NULL,
};

static INLINE void OsTickResume(U32 sleepTicks)
{
    U32 cpuid = OsGetCoreID();
    if (sleepTicks > g_pmMgr.sleepTime[cpuid]) {
        sleepTicks -= g_pmMgr.sleepTime[cpuid];
    } else {
        sleepTicks = 0;
    }
    OsSysTimeUpdate(sleepTicks);
}

static void OsDeepSleepResume(void)
{
    DEEPOPS_CALL_FUNC_VOID(resumeFromReset);
    PM_ATOMIC_SET(&g_pmMgr.resumeSleepCores, OS_ALLCORES_MASK);

#if (OS_MAX_CORE_NUM > 1)
    /* release_secondary_cores(); */
#endif
    OsLowpowerRestoreRegister();
}

static INLINE void OsEnterDeepSleepMainCore(void)
{
    PM_ATOMIC_ADD(&g_pmMgr.deepSleepCores, 1);
    g_deepSleepOps.suspendPreConfig();

    if (PM_ATOMIC_READ(&g_pmMgr.deepSleepCores) == OS_MAX_CORE_NUM && g_deepSleepOps.couldDeepSleep()) {
        g_deepSleepOps.suspendDevice();
        g_pmRunOps.setWakeUpTimer(g_pmMgr.sleepTime[0]);
        g_lowpowerResumeFromImg = LOWPOWER_COLD_RESET;
        OsLowpowerSaveRegister();

        if (g_lowpowerResumeFromImg == LOWPOWER_COLD_RESET) {
            g_lowpowerResumeFromImg = LOWPOWER_DEEP_SLEEP_RESET;
            CALL_RUN_OPS_FUNC_NO_RETURN(contextSave);
            g_pmRunOps.enterDeepSleep();
            g_deepSleepOps.rollback();
        }
        g_deepSleepOps.resumeDevice();
        U32 sleepTicks = g_pmRunOps.withdrawWakeUpTimer();
        OsSysTimeUpdate(sleepTicks);
    } else {
#if defined(OS_OPTION_TICKLESS)
        OsTicklessOpen();
#endif
        g_pmRunOps.enterLightSleep(0);
    }
    g_deepSleepOps.resumePostConfig();
    PM_ATOMIC_SUB(&g_pmMgr.deepSleepCores, 1);
}

static INLINE void OsEnterSleepMode(void)
{
#if (OS_MAX_CORE_NUM > 1)
    U32 currCpuid = OsGetCoreID();
    if (currCpuid == 0) {
        OsEnterDeepSleepMainCore();
        return;
    }

    U32 cpuMask = 1 << currCpuid;
    PM_ATOMIC_ADD(&g_pmMgr.deepSleepCores, 1);
    OsLowpowerSaveRegister();
    if (PM_ATOMIC_READ(&g_pmMgr.resumeSleepCores) & cpuMask) {
        S32 val;
        do {
            val = PM_ATOMIC_READ(&g_pmMgr.resumeSleepCores);
        } while (!PM_ATOMIC_CMPXCHG(&g_pmMgr.resumeSleepCores, (U32)val & (~cpuMask), (U32)val));
        g_deepSleepOps.otherCoreResume();
        U32 sleepTicks = g_pmRunOps.withdrawWakeUpTimer();
        OsTickResume(sleepTicks);
    } else {
        if (PM_ATOMIC_READ(&g_pmMgr.deepSleepCores) == (S32)OS_MAX_CORE_NUM) {
            /* LOS_MpSchedule(1 << 0); */
        }
#if defined(OS_OPTION_TICKLESS)
        OsTicklessOpen();
#endif
        g_pmRunOps.enterLightSleep(0);
    }
    PM_ATOMIC_SUB(&g_pmMgr.deepSleepCores, 1);
#else
    OsEnterDeepSleepMainCore();
#endif
}

static INLINE void OsSystemSuspend(enum PRT_IntermitMode *mode)
{
    switch (*mode) {
        case PRT_INTERMIT_SHUTDOWN:
            /* fall through */
        case PRT_INTERMIT_STANDBY:
            /* fall through */
        case PRT_INTERMIT_DEEP_SLEEP:
            if (g_pmRunOps.enterDeepSleep != NULL) {
                OsEnterSleepMode();
                break;
            }
            /* fall through */
        default:
            *mode = PRT_INTERMIT_LIGHT_SLEEP;
            break;
    }
}
#endif /* OS_OPTION_DEEPSLEEP */

static void OsLowpowerLightSleep(U32 mode, U32 cpuid, U32 sleepTicks)
{
    if (g_pmRunOps.preConfig != NULL) {
        sleepTicks = g_pmRunOps.getSleepTime();
    }
    if (sleepTicks > 1) {
        g_pmMgr.sleepMode[cpuid] = (U8)(mode & 0x0FF);
        g_pmMgr.sleepTime[cpuid] = sleepTicks;
#if defined(OS_OPTION_TICKLESS)
        OsTicklessOpen();
#endif
        if (mode == PRT_INTERMIT_LIGHT_SLEEP && g_pmRunOps.enterLightSleep != NULL) {
            g_pmRunOps.enterLightSleep(sleepTicks);
        } else {
            OsAsmWfi();
        }
    } else {
        g_pmMgr.sleepMode[cpuid] = PRT_INTERMIT_NONE;
        g_pmMgr.sleepTime[cpuid] = 0;
        OsAsmWfi();
    }
}

static void OsLowpowerDeepSleep(enum PRT_IntermitMode *mode, U32 cpuid, U32 sleepTicks)
{
#if defined(OS_OPTION_DEEPSLEEP)
    if (g_pmRunOps.enterDeepSleep == NULL) {
        *mode = PRT_INTERMIT_LIGHT_SLEEP;
    } else {
        *mode = PRT_INTERMIT_DEEP_SLEEP;
        g_pmMgr.sleepMode[cpuid] = (U8)*mode;
        g_pmMgr.sleepTime[cpuid] = sleepTicks;
        OsSystemSuspend(mode);
    }
#else
    (void)cpuid;
    (void)sleepTicks;
    *mode = PRT_INTERMIT_LIGHT_SLEEP;
#endif
}

static void OsLowpowerProcess(void)
{
#if defined(OS_OPTION_RUNSTOP)
    if (OsRunstopWowSysDoneFlagGet() == OS_STORE_SYSTEM) {
        OsRunstopStoreSystemInfoBeforeSuspend();
    }
#endif
    if (g_pmRunOps.changeFreq != NULL) {
        OsChangeFreq();
    }

    /* Without a sleep-budget source (no tickless) and no deep sleep, the power
     * manager has no managed-sleep decision to make: the runstop bridge and the
     * freq change above are the only useful idle work. Return and let the idle
     * task's coreSleep (wfi) take the CPU. This mirrors LiteOs, which only runs
     * the managed-sleep path below when tickless is on -- lowpower and tickless
     * are always paired there, so getSleepTime() returns a real budget and the
     * work-mode branch is hit only when the next event is very near (rare).
     * Running that path without tickless makes getSleepTime() always 0, hitting
     * work-mode (and its PRINT_WARN) every tick. */
#if defined(OS_OPTION_TICKLESS) || defined(OS_OPTION_DEEPSLEEP)
    uintptr_t intSave = OsIntLock();
    PRT_TaskLock();
    RUNOPS_CALL_FUNC_VOID(preConfig);
    U32 cpuid = OsGetCoreID();
    U32 sleepTicks = g_pmRunOps.getSleepTime();
    if (sleepTicks <= g_pmMgr.minSleepTicks || PRT_PowerMgrGetDeepSleepVoteCount() != 0) {
        g_pmMgr.sleepMode[cpuid] = PRT_INTERMIT_NONE;
        g_pmMgr.sleepTime[cpuid] = 0;

        RUNOPS_CALL_FUNC_VOID(postConfig);

#if defined(OS_OPTION_TICKLESS)
        OsTicklessOpen();
#endif
        OsAsmWfi();

        PRINT_WARN("Application is running in work mode, powermgr do not process.\n");
    } else {
        if (sleepTicks > g_pmMgr.maxSleepCount) {
            sleepTicks = g_pmMgr.maxSleepCount;
        }
        U32 mode = g_pmRunOps.selectSleepMode(sleepTicks);
        if (mode >= PRT_INTERMIT_DEEP_SLEEP) {
            g_pmMgr.sleepTime[cpuid] = g_pmRunOps.withdrawWakeUpTimer();
            OsLowpowerDeepSleep((enum PRT_IntermitMode *)&mode, cpuid, sleepTicks);
        }

        RUNOPS_CALL_FUNC_VOID(postConfig);

        if (mode < PRT_INTERMIT_DEEP_SLEEP) {
            OsLowpowerLightSleep(mode, cpuid, sleepTicks);
        }
    }

    PRT_TaskUnlock();
    OsIntRestore(intSave);
#endif
}

static void OsLowpowerWakeupFromReset(void)
{
#if defined(OS_OPTION_RUNSTOP)
    if (g_lowpowerResumeFromImg == LOWPOWER_RUNSTOP_RESET) {
        OsRunstopSystemWakeup();
        return;
    }
#endif
#if defined(OS_OPTION_DEEPSLEEP)
    if (g_lowpowerResumeFromImg == LOWPOWER_DEEP_SLEEP_RESET) {
        OsDeepSleepResume();
    }
#endif
}

static void OsLowpowerWakeupFromInterrupt(U32 intNum)
{
#if defined(OS_OPTION_TICKLESS)
    OsTicklessUpdate(intNum);
#else
    (void)intNum;
#endif
}

static void OsChangeFreq(void)
{
    U32 freq;
    bool ret;

    do {
        ret = PM_ATOMIC_CMPXCHG(&g_pmMgr.freeLock, LOCK_ON, LOCK_OFF);
        if (!ret) {
            return;
        }
        freq = (U32)PM_ATOMIC_READ(&g_pmMgr.freqPending);
        if (freq != (U32)PM_ATOMIC_READ(&g_pmMgr.freq)) {
            g_pmRunOps.changeFreq((U8)freq);
            PM_ATOMIC_SET(&g_pmMgr.freq, (S32)freq);
        }
        PM_ATOMIC_SET(&g_pmMgr.freeLock, LOCK_OFF);
    } while (FreqHigher((enum PRT_FreqMode)PM_ATOMIC_READ(&g_pmMgr.freqPending), (enum PRT_FreqMode)freq));
}

static void OsLowpowerChangeFreq(enum PRT_FreqMode freq)
{
    TRACE_FUNC_CALL();
    if (g_pmRunOps.changeFreq == NULL) {
        PRINT_DEBUG("freq function is null.\n");
        return;
    }

    if ((U32)freq >= PRT_SYS_FREQ_MAX) {
        PRINT_DEBUG("freq(%d) is invalid.\n", (U32)freq);
        return;
    }

    PM_ATOMIC_SET(&g_pmMgr.freqPending, (S32)freq);

    if (FreqHigher((enum PRT_FreqMode)PM_ATOMIC_READ(&g_pmMgr.freqPending),
                   (enum PRT_FreqMode)PM_ATOMIC_READ(&g_pmMgr.freq)) && g_pmRunOps.changeFreq != NULL) {
        OsChangeFreq();
    }
}

static void OsLowpowerDeepSleepVoteBegin(void)
{
    TRACE_FUNC_CALL();
    PM_ATOMIC_INC(&g_pmMgr.sleepVoteCount);
    LOS_ASSERT(PM_ATOMIC_READ(&g_pmMgr.sleepVoteCount) > 0);
}

static void OsLowpowerDeepSleepVoteEnd(void)
{
    TRACE_FUNC_CALL();
    PM_ATOMIC_DEC(&g_pmMgr.sleepVoteCount);
    LOS_ASSERT(PM_ATOMIC_READ(&g_pmMgr.sleepVoteCount) >= 0);
}

static void OsLowpowerDeepSleepVoteDelay(U32 delayTicks)
{
    TRACE_FUNC_CALL();
    (void)delayTicks;
}

static void OsLowpowerRegisterExternalVoter(LowpowerExternalVoterHandle callback)
{
    TRACE_FUNC_CALL();
    g_pmMgr.exVoterHandle = callback;
}

static U32 OsLowpowerGetDeepSleepVoteCount(void)
{
    if (g_pmMgr.exVoterHandle == NULL) {
        return (U32)PM_ATOMIC_READ(&g_pmMgr.sleepVoteCount);
    } else {
        return (U32)PM_ATOMIC_READ(&g_pmMgr.sleepVoteCount) + (U32)g_pmMgr.exVoterHandle();
    }
}

static struct PowerMgrOps g_pmOps = {
    .process = OsLowpowerProcess,
    .wakeupFromReset = OsLowpowerWakeupFromReset,
    .resumeFromInterrupt = OsLowpowerWakeupFromInterrupt,
    .changeFreq = OsLowpowerChangeFreq,
    .deepSleepVoteBegin = OsLowpowerDeepSleepVoteBegin,
    .deepSleepVoteEnd = OsLowpowerDeepSleepVoteEnd,
    .deepSleepVoteDelay = OsLowpowerDeepSleepVoteDelay,
    .registerExternalVoter = OsLowpowerRegisterExternalVoter,
    .getDeepSleepVoteCount = OsLowpowerGetDeepSleepVoteCount,
    .getSleepMode = NULL,
    .setSleepMode = NULL,
};

#define FORCE_NULL_CALLBACK (void *)0x3f3f3f3f

#define ASSIGN_MEMBER(lhs, rhs, member)             \
    do {                                            \
        if ((rhs)->member == FORCE_NULL_CALLBACK) { \
            (lhs)->member = NULL;                   \
        } else if ((rhs)->member != NULL) {         \
            (lhs)->member = (rhs)->member;          \
        }                                           \
    } while (0)

#define ASSIGN_MEMBER_NOT_NULL(lhs, rhs, member)                              \
    do {                                                                      \
        ASSIGN_MEMBER(lhs, rhs, member);                                      \
        if ((lhs)->member == NULL) {                                          \
            PRINT_ERR("%s must be non-null.\n", __FUNCTION__);                \
            while (1) { }                                                     \
        }                                                                     \
    } while (0)

OS_SEC_BSS static struct PowerMgrStats g_pmStats;

void PRT_PowerMgrStatsGet(struct PowerMgrStats *stats)
{
    if (stats != NULL) {
        *stats = g_pmStats;
    }
}

void OsPmSetReady(void)
{
    /* Nothing extra to do for the light-sleep path; reserved for BSP readiness. */
}

/* Compatibility aliases used by the existing sd3403 BSP. */
void OsPmInit(const struct PowerMgrParameter *para)
{
    PRT_PowerMgrInit(para);
}

void OsPmProcess(void)
{
    OsLowpowerProcess();
}

void PRT_PowerMgrInit(const struct PowerMgrParameter *para)
{
    const struct PowerMgrRunOps *runOps = NULL;
#if defined(OS_OPTION_DEEPSLEEP)
    const struct PowerMgrDeepSleepOps *deepSleepOps = NULL;
#endif
    if (para != NULL) {
        runOps = &para->runOps;
#if defined(OS_OPTION_DEEPSLEEP)
        deepSleepOps = &para->deepSleepOps;
#endif
        g_pmMgr.minSleepTicks = para->config.minLightSleepTicks;
        g_pmMgr.maxSleepCount = para->config.maxDeepSleepTicks;
        g_pmMgr.minDeepSleepTicks = para->config.minDeepSleepTicks;
    }

    PM_ATOMIC_SET(&g_pmMgr.resumeSleepCores, 0);
    g_pmMgr.lock.rawLock = OS_SPINLOCK_UNLOCK;
    PM_ATOMIC_SET(&g_pmMgr.freeLock, LOCK_OFF);

    if (runOps != NULL) {
        ASSIGN_MEMBER(&g_pmRunOps, runOps, changeFreq);
        ASSIGN_MEMBER(&g_pmRunOps, runOps, enterLightSleep);
#if defined(OS_OPTION_DEEPSLEEP)
        ASSIGN_MEMBER_NOT_NULL(&g_pmRunOps, runOps, enterDeepSleep);
        ASSIGN_MEMBER_NOT_NULL(&g_pmRunOps, runOps, setWakeUpTimer);
        ASSIGN_MEMBER_NOT_NULL(&g_pmRunOps, runOps, withdrawWakeUpTimer);
#else
        ASSIGN_MEMBER(&g_pmRunOps, runOps, enterDeepSleep);
        ASSIGN_MEMBER(&g_pmRunOps, runOps, setWakeUpTimer);
        ASSIGN_MEMBER(&g_pmRunOps, runOps, withdrawWakeUpTimer);
#endif
        ASSIGN_MEMBER_NOT_NULL(&g_pmRunOps, runOps, getSleepTime);
        ASSIGN_MEMBER_NOT_NULL(&g_pmRunOps, runOps, selectSleepMode);
        ASSIGN_MEMBER(&g_pmRunOps, runOps, preConfig);
        ASSIGN_MEMBER(&g_pmRunOps, runOps, postConfig);
    }

#if defined(OS_OPTION_DEEPSLEEP)
    if (deepSleepOps != NULL) {
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, couldDeepSleep);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, systemWakeup);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, suspendPreConfig);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, suspendDevice);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, rollback);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, resumeDevice);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, resumePostConfig);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, resumeCallBack);
        ASSIGN_MEMBER(&g_deepSleepOps, deepSleepOps, otherCoreResume);
    }
#endif
    PRT_LowpowerInit(&g_pmOps);
}
