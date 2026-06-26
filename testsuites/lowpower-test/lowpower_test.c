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
 * Description: Low-power / runstop / powermgr on-board test (single mica instance,
 *              Linux-transparent).
 *
 *              The runstop flow runs entirely inside one instance:
 *                save CPU context -> save a REAL RAM image (.data/.bss/regs/tick)
 *                to the spare mcs_mem carveout at WOW_IMG_ADDR -> enter (simulated)
 *                deep sleep = WFI loop woken by the hardware tick -> wake ->
 *                withdraw -> compensate tick -> signal -> inline resume.
 *              No mica restart, no reset vector, no PSCI, Linux sees the instance
 *              running throughout. Real (power-gating) deep sleep is a pluggable
 *              hook the customer supplies later; the default is wfi(), exactly as
 *              in LiteOs (OsEnterDeepSleepDefault).
 */
#include <stdio.h>
#include <string.h>
#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_task.h"
#include "prt_sem.h"
#include "prt_hwi.h"
#include "prt_tick.h"
#include "prt_lowpower.h"
#include "prt_lowpower_impl.h"
#include "prt_lowpower_context.h"
#include "prt_runstop.h"
#include "prt_timer.h"
#include "prt_module.h"
#include "lowpower_test.h"
#include "cpu_config.h"

extern U32 PRT_Printf(const char *format, ...);

extern U8 __data_start[];
extern U8 __data_end[];
extern uintptr_t __bss_start__;
extern uintptr_t __bss_end__;

#define RUNSTOP_RESULT_FLASH 0x02U
#define TEST_MARKER_VAL      0xA5A55A5AU
#define TEST_DATA_VAL        0x123456789ABCDEF0ULL
#define SR_CONTEXT_BYTES     (34U * 8U)   /* g_lowpowerSaveSRContext[34] of U64 */
#define RUN_DELAY_TICKS      300U         /* let the system run before imaging */
#define SUSPEND_TICKS        30U          /* simulated deep-sleep length (~300ms) */
#define SWTMR_INTERVAL_MS    200U         /* one-shot sw timer length */
#define SWTMR_EXPECTED_TICKS 20U          /* 200ms @100Hz = 20 ticks */
#define SWTMR_WAIT_TICKS     120U         /* wait well past the sw timer */
#define SWTMR_LOOP_PERIOD_MS 100U         /* loop sw timer period */
#define SWTMR_LOOP_EXPECTED  10U          /* 100ms @100Hz = 10 ticks */
#define SWTMR_LOOP_PERIODS   5U           /* expect >=5 fires */
#define SWTMR_LOOP_WAIT_TICKS 60U         /* wait well past 5 periods */
#define SWTMR_LOOP_RECORD    8U           /* record first 8 fire ticks */

static volatile U32 g_testMarker;                 /* lives in .bss */
static U64 g_testDataVar = TEST_DATA_VAL;         /* lives in .data */
static volatile bool g_saveImageCalled = false;
static volatile U32 g_swtmrFired = 0;
static volatile U64 g_swtmrFireTick = 0;
static volatile U32 g_swtmrLoopCount = 0;
static volatile U64 g_swtmrLoopFireTicks[SWTMR_LOOP_RECORD];

/* Layout at WOW_IMG_ADDR:
 *   [ WowShmHeader | .data | .bss | register-context(272B) | tick(8B) ] */
static inline U8 *TestPayloadAfterHeader(void)
{
    return (U8 *)WOW_IMG_ADDR + sizeof(WowShmHeader);
}

static U32 TestValidateImage(void)
{
    WowShmHeader *hdr = (WowShmHeader *)WOW_IMG_ADDR;
    return (hdr->magic == RUNSTOP_MAGIC) ? RUNSTOP_OK : RUNSTOP_FAIL;
}

static U32 TestSaveImage(const RunstopImageInfo *info)
{
    WowShmHeader *hdr = (WowShmHeader *)WOW_IMG_ADDR;
    U8 *p = TestPayloadAfterHeader();
    if (info == NULL) {
        return RUNSTOP_FAIL;
    }
    hdr->magic = RUNSTOP_MAGIC;
    hdr->imageSize = info->imageSize;
    hdr->dataSize = info->dataSize;
    hdr->bssSize = info->bssSize;
    hdr->resumeFlag = RUNSTOP_RESUME;

    /* Genuine snapshot of the running system's RAM + register context + tick. */
    (void)memcpy(p, (void *)(uintptr_t)info->dataStart, (size_t)info->dataSize);
    (void)memcpy(p + info->dataSize, (void *)(uintptr_t)info->bssStart, (size_t)info->bssSize);
    (void)memcpy(p + info->dataSize + info->bssSize,
                 (void *)(uintptr_t)info->srContextAddr, (size_t)info->srContextSize);
    *(U64 *)(p + info->dataSize + info->bssSize + SR_CONTEXT_BYTES) = info->tickCount;

    g_saveImageCalled = true;
    return RUNSTOP_OK;
}

/* Restore RAM from the persisted image. Only used on a real (power-gating) deep
 * sleep's reset-resume path; NOT called in the inline WFI simulation. */
static void TestRestoreImage(void)
{
    WowShmHeader *hdr = (WowShmHeader *)WOW_IMG_ADDR;
    U8 *p = TestPayloadAfterHeader();
    if (hdr->magic != RUNSTOP_MAGIC) {
        return;
    }
    (void)memcpy((void *)(uintptr_t)__data_start, p, (size_t)hdr->dataSize);
    (void)memcpy((void *)(uintptr_t)&__bss_start__, p + hdr->dataSize, (size_t)hdr->bssSize);
}

static const struct RunstopStorageOps g_testStorageOps = {
    .getImageInfo = NULL,
    .validateImage = TestValidateImage,
    .saveImage = TestSaveImage,
    .restoreImage = TestRestoreImage,
};

static U32 LowpowerTestPowerMgr(void)
{
    U32 failures = 0;
    U32 cnt;

    PRT_Printf("[LOWPOWER] powermgr: init with default ops\n");
    PRT_PowerMgrInit(NULL);

    cnt = PRT_PowerMgrGetDeepSleepVoteCount();
    PRT_Printf("[LOWPOWER] powermgr: initial vote count=%u\n", cnt);
    if (cnt != 0) { failures++; }

    PRT_PowerMgrDeepSleepVoteBegin();
    PRT_PowerMgrDeepSleepVoteBegin();
    cnt = PRT_PowerMgrGetDeepSleepVoteCount();
    PRT_Printf("[LOWPOWER] powermgr: vote count after 2x begin=%u\n", cnt);
    if (cnt != 2) { failures++; }

    PRT_PowerMgrDeepSleepVoteEnd();
    cnt = PRT_PowerMgrGetDeepSleepVoteCount();
    PRT_Printf("[LOWPOWER] powermgr: vote count after 1x end=%u\n", cnt);
    if (cnt != 1) { failures++; }
    PRT_PowerMgrDeepSleepVoteEnd();
    return failures;
}

static U32 LowpowerTestTickless(void)
{
#if defined(OS_OPTION_TICKLESS)
    PRT_TicklessSetTickIrqNum(TEST_CLK_INT);
    PRT_Printf("[LOWPOWER] tickless: OsSleepTicksGet=%u\n", OsSleepTicksGet());
#else
    PRT_Printf("[LOWPOWER] tickless: option disabled (native non-SMP gap)\n");
#endif
    return 0;
}

/* One-shot software timer: exercises the swtmr sortlink (start/expire/delete).
 * Guards the tickless sortlink port: if the swtmr scan breaks, this fires late
 * or not at all. */
static void TestSwtmrCallback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    (void)tmrHandle;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    g_swtmrFireTick = PRT_TickGetCount();
    g_swtmrFired = 1;
}

static U32 LowpowerTestSwtmr(void)
{
    struct TimerCreatePara para = {0};
    TimerHandle handle = 0;
    U32 ret;
    U32 failures = 0;
    U64 tickBefore, tickAfter;

    para.type = OS_TIMER_SOFTWARE;
    para.mode = OS_TIMER_ONCE;
    para.interval = SWTMR_INTERVAL_MS;
    para.callBackFunc = TestSwtmrCallback;

    ret = PRT_TimerCreate(&para, &handle);
    if (ret != OS_OK) {
        PRT_Printf("[LOWPOWER] swtmr: PRT_TimerCreate FAILED ret=%u\n", ret);
        return 1;
    }

    g_swtmrFired = 0;
    g_swtmrFireTick = 0;
    tickBefore = PRT_TickGetCount();
    PRT_Printf("[LOWPOWER] swtmr: start one-shot %ums (%u ticks expected)\n",
               SWTMR_INTERVAL_MS, SWTMR_EXPECTED_TICKS);

    ret = PRT_TimerStart(OS_MID_SWTMR, handle);
    if (ret != OS_OK) {
        PRT_Printf("[LOWPOWER] swtmr: PRT_TimerStart FAILED ret=%u\n", ret);
        failures++;
    }

    PRT_TaskDelay(SWTMR_WAIT_TICKS);

    tickAfter = PRT_TickGetCount();
    if (!g_swtmrFired) {
        PRT_Printf("[LOWPOWER] swtmr: timer did NOT fire FAILED (waited %llu ticks)\n",
                   (unsigned long long)(tickAfter - tickBefore));
        failures++;
    } else {
        U64 fired = g_swtmrFireTick - tickBefore;
        PRT_Printf("[LOWPOWER] swtmr: fired at +%llu ticks (expected ~%u)\n",
                   (unsigned long long)fired, SWTMR_EXPECTED_TICKS);
        if (fired < (SWTMR_EXPECTED_TICKS - 5) || fired > (SWTMR_EXPECTED_TICKS + 5)) {
            PRT_Printf("[LOWPOWER] swtmr: fire timing off FAILED\n");
            failures++;
        }
    }

    (void)PRT_TimerDelete(OS_MID_SWTMR, handle);
    return failures;
}

/* Loop software timer: run several periods and check each fires ~period ticks
 * after the previous. Guards the tickless swtmr scan: a wrong scan (e.g. the
 * cursor-catch-up that re-inserts with a stale cursor) makes loop timers drift
 * or fire early. */
static void TestSwtmrLoopCallback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    (void)tmrHandle;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    U32 idx = g_swtmrLoopCount;
    if (idx < SWTMR_LOOP_RECORD) {
        g_swtmrLoopFireTicks[idx] = PRT_TickGetCount();
    }
    g_swtmrLoopCount = idx + 1;
}

static U32 LowpowerTestSwtmrLoop(void)
{
    struct TimerCreatePara para = {0};
    TimerHandle handle = 0;
    U32 ret;
    U32 failures = 0;
    U64 tickBefore;
    U32 i;

    para.type = OS_TIMER_SOFTWARE;
    para.mode = OS_TIMER_LOOP;
    para.interval = SWTMR_LOOP_PERIOD_MS;
    para.callBackFunc = TestSwtmrLoopCallback;

    ret = PRT_TimerCreate(&para, &handle);
    if (ret != OS_OK) {
        PRT_Printf("[LOWPOWER] swtmr-loop: PRT_TimerCreate FAILED ret=%u\n", ret);
        return 1;
    }

    g_swtmrLoopCount = 0;
    tickBefore = PRT_TickGetCount();
    PRT_Printf("[LOWPOWER] swtmr-loop: start %ums period (%u ticks), expect >=%u fires\n",
               SWTMR_LOOP_PERIOD_MS, SWTMR_LOOP_EXPECTED, SWTMR_LOOP_PERIODS);
    ret = PRT_TimerStart(OS_MID_SWTMR, handle);
    if (ret != OS_OK) {
        PRT_Printf("[LOWPOWER] swtmr-loop: PRT_TimerStart FAILED ret=%u\n", ret);
        failures++;
    }

    PRT_TaskDelay(SWTMR_LOOP_WAIT_TICKS);
    (void)PRT_TimerDelete(OS_MID_SWTMR, handle);

    if (g_swtmrLoopCount < SWTMR_LOOP_PERIODS) {
        PRT_Printf("[LOWPOWER] swtmr-loop: only %u fires (expected >=%u) FAILED\n",
                   g_swtmrLoopCount, SWTMR_LOOP_PERIODS);
        failures++;
        return failures;
    }

    PRT_Printf("[LOWPOWER] swtmr-loop: %u fires, intervals:", g_swtmrLoopCount);
    for (i = 1; i < SWTMR_LOOP_RECORD && i < g_swtmrLoopCount; i++) {
        U64 interval = g_swtmrLoopFireTicks[i] - g_swtmrLoopFireTicks[i - 1];
        PRT_Printf(" %llu", (unsigned long long)interval);
        if (interval < (SWTMR_LOOP_EXPECTED - 2) || interval > (SWTMR_LOOP_EXPECTED + 2)) {
            failures++;
        }
    }
    PRT_Printf(" (expected ~%u)\n", SWTMR_LOOP_EXPECTED);

    U64 first = g_swtmrLoopFireTicks[0] - tickBefore;
    PRT_Printf("[LOWPOWER] swtmr-loop: first fire at +%llu (expected ~%u)\n",
               (unsigned long long)first, SWTMR_LOOP_EXPECTED);
    if (first < (SWTMR_LOOP_EXPECTED - 2) || first > (SWTMR_LOOP_EXPECTED + 2)) {
        failures++;
    }
    return failures;
}

static U32 LowpowerTestRunstop(void)
{
    U32 ret;
    U32 failures = 0;
    U64 tickBefore;
    U64 tickAfter;

    PRT_Printf("[LOWPOWER] runstop: backend=0x%p (spare mcs_mem, Linux-transparent)\n",
               (void *)(uintptr_t)WOW_IMG_ADDR);
    OsRunstopStorageOpsRegister(&g_testStorageOps);
    /* Default low-power ops: WFI loop woken by the hardware tick. A customer with
     * real power-gating deep sleep registers its own enterDeepSleep + wake timer. */
    OsRunstopSetSleepTicks(SUSPEND_TICKS);
    ret = OsRunstopInit();
    if (ret != RUNSTOP_OK) {
        PRT_Printf("[LOWPOWER] runstop: OsRunstopInit FAILED ret=%u\n", ret);
        return 1;
    }
    if (OsRunstopResumeCheck() != RUNSTOP_COLD_BOOT) {
        PRT_Printf("[LOWPOWER] runstop: resume check (cold boot) FAILED\n");
        failures++;
    }

    /* Seed markers so the saved image carries known, checkable runtime values. */
    g_testMarker = TEST_MARKER_VAL;

    PRT_Printf("[LOWPOWER] runstop: make image (save real .data/.bss/regs/tick -> WFI %u ticks -> resume)\n",
               SUSPEND_TICKS);
    g_saveImageCalled = false;
    tickBefore = PRT_TickGetCount();
    ret = OsRunstopMakeImage();
    tickAfter = PRT_TickGetCount();
    if (ret != RUNSTOP_RESULT_FLASH) {
        PRT_Printf("[LOWPOWER] runstop: OsRunstopMakeImage FAILED ret=%u (expected %u)\n",
                   ret, RUNSTOP_RESULT_FLASH);
        failures++;
    } else {
        PRT_Printf("[LOWPOWER] runstop: OsRunstopMakeImage OK, suspended ~%llu ticks (inline resume)\n",
                   (unsigned long long)(tickAfter - tickBefore));
        if ((tickAfter - tickBefore) < (SUSPEND_TICKS - 5) || (tickAfter - tickBefore) > (SUSPEND_TICKS + 10)) {
            PRT_Printf("[LOWPOWER] runstop: suspend duration off FAILED\n");
            failures++;
        }
    }
    if (!g_saveImageCalled) {
        PRT_Printf("[LOWPOWER] runstop: saveImage callback NOT invoked FAILED\n");
        failures++;
    }
    if (TestValidateImage() != RUNSTOP_OK) {
        PRT_Printf("[LOWPOWER] runstop: validateImage FAILED\n");
        failures++;
    }
    return failures;
}

/* Read the persisted image back and verify it captured the live RAM values. */
static U32 LowpowerTestReadback(void)
{
    WowShmHeader *hdr = (WowShmHeader *)WOW_IMG_ADDR;
    U8 *p = TestPayloadAfterHeader();
    U32 failures = 0;
    U64 bssOff;
    U64 dataOff;
    U32 savedMarker;
    U64 savedData;
    U64 savedTick;

    if (hdr->magic != RUNSTOP_MAGIC) {
        PRT_Printf("[LOWPOWER] readback: magic mismatch 0x%x FAILED\n", hdr->magic);
        return 1;
    }
    PRT_Printf("[LOWPOWER] readback: header dataSize=%llu bssSize=%llu\n",
               (unsigned long long)hdr->dataSize, (unsigned long long)hdr->bssSize);

    bssOff = (U64)(uintptr_t)&g_testMarker - (U64)(uintptr_t)&__bss_start__;
    savedMarker = *(U32 *)(p + hdr->dataSize + bssOff);
    PRT_Printf("[LOWPOWER] readback: saved .bss marker=0x%x live=0x%x\n", savedMarker, g_testMarker);
    if (savedMarker != g_testMarker) {
        PRT_Printf("[LOWPOWER] readback: .bss marker mismatch FAILED\n");
        failures++;
    }

    dataOff = (U64)(uintptr_t)&g_testDataVar - (U64)(uintptr_t)__data_start;
    savedData = *(U64 *)(p + dataOff);
    PRT_Printf("[LOWPOWER] readback: saved .data var=0x%llx live=0x%llx\n",
               (unsigned long long)savedData, (unsigned long long)g_testDataVar);
    if (savedData != g_testDataVar) {
        PRT_Printf("[LOWPOWER] readback: .data var mismatch FAILED\n");
        failures++;
    }

    savedTick = *(U64 *)(p + hdr->dataSize + hdr->bssSize + SR_CONTEXT_BYTES);
    PRT_Printf("[LOWPOWER] readback: saved tick=%llu (captured at suspend) current=%llu\n",
               (unsigned long long)savedTick, (unsigned long long)PRT_TickGetCount());
    if (savedTick == 0) {
        PRT_Printf("[LOWPOWER] readback: saved tick is zero FAILED\n");
        failures++;
    }
    return failures;
}

void lowpower_test(void)
{
    U32 failures = 0;

    PRT_Printf("\n========== [LOWPOWER] test start ==========\n");
    OsLowpowerContextInit();
    failures += LowpowerTestPowerMgr();
    failures += LowpowerTestTickless();
    failures += LowpowerTestSwtmr();
    failures += LowpowerTestSwtmrLoop();

    /* Let the system run so the saved image reflects real runtime state. */
    PRT_Printf("[LOWPOWER] runstop: letting system run %u ticks before imaging...\n", RUN_DELAY_TICKS);
    PRT_TaskDelay(RUN_DELAY_TICKS);

    failures += LowpowerTestRunstop();
    failures += LowpowerTestReadback();

    PRT_Printf("========== [LOWPOWER] test end: %u failure(s) ==========\n", failures);
    if (failures == 0) {
        PRT_Printf("[LOWPOWER] RESULT: PASS\n");
    } else {
        PRT_Printf("[LOWPOWER] RESULT: FAIL\n");
    }
}
