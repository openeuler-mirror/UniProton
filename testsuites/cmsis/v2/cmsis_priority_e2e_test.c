/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS thread-priority end-to-end test.
 *
 * Test method:
 * 1. Create a high-priority CMSIS thread and a low-priority CMSIS thread.
 * 2. The high-priority thread records whether the low-priority thread has run
 *    at the moment the high-priority thread starts executing.
 * 3. The low-priority thread deliberately delays before marking itself as run.
 * 4. The test passes only if both threads run and the high-priority thread runs
 *    before the low-priority thread marks itself, proving CMSIS priority values
 *    affect scheduling order.
 *
 * Sequence diagram:
 *   Create high-priority thread      Create low-priority thread
 *   ---------------------------      --------------------------
 *   high thread runs first
 *   records lowRan == 0
 *                                    low thread later sets lowRan = 1
 *   PASS if both ran and high observed lowRan == 0
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Set by the high-priority thread once it starts running. */
    volatile uint32_t highRan;
    /* Set by the low-priority thread once it starts running. */
    volatile uint32_t lowRan;
    /* Snapshot of lowRan taken by the high-priority thread at its start. */
    volatile uint32_t lowRanWhenHighRan;
} PriorityE2ECtx;

static int CmsisPriorityE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] prioE2E %s=0x%x\n", detail, value);
    return 1;
}

static void HighPrioTask(void *arg)
{
    PriorityE2ECtx *ctx = (PriorityE2ECtx *)arg;

    ctx->lowRanWhenHighRan = ctx->lowRan;
    ctx->highRan = 1;
    for (;;) {
        (void)osDelay(10);
    }
}

static void LowPrioTask(void *arg)
{
    PriorityE2ECtx *ctx = (PriorityE2ECtx *)arg;

    (void)osDelay(5);
    ctx->lowRan = 1;
    for (;;) {
        (void)osDelay(10);
    }
}

int CmsisPriorityE2ETest(void)
{
    PriorityE2ECtx ctx = {0};
    osThreadAttr_t highAttr = {0};
    osThreadAttr_t lowAttr = {0};
    osThreadId_t highTid;
    osThreadId_t lowTid;
    uint32_t wait;

    highAttr.name = "highPrio";
    highAttr.priority = osPriorityHigh;
    highAttr.stack_size = 0x1000;
    highTid = osThreadNew(HighPrioTask, &ctx, &highAttr);
    if (highTid == NULL) {
        return CmsisPriorityE2EFail("highNew", 0);
    }

    lowAttr.name = "lowPrio";
    lowAttr.priority = osPriorityLow;
    lowAttr.stack_size = 0x1000;
    lowTid = osThreadNew(LowPrioTask, &ctx, &lowAttr);
    if (lowTid == NULL) {
        (void)osThreadTerminate(highTid);
        return CmsisPriorityE2EFail("lowNew", 0);
    }

    for (wait = 0; wait < 50 && (ctx.highRan == 0 || ctx.lowRan == 0); wait++) {
        (void)osDelay(1);
    }

    (void)osThreadTerminate(highTid);
    (void)osThreadTerminate(lowTid);

    if (ctx.highRan != 1 || ctx.lowRan != 1) {
        return CmsisPriorityE2EFail("notRan", ctx.highRan * 10 + ctx.lowRan);
    }
    if (ctx.lowRanWhenHighRan != 0) {
        return CmsisPriorityE2EFail("prioWrong", ctx.lowRanWhenHighRan);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] prioE2E\n");
    return 0;
}
