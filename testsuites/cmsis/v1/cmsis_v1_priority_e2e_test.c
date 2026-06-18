/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 priority end-to-end test.
 *
 * Test method:
 * 1. Create one high-priority and one low-priority thread.
 * 2. Both threads block on the same CMSIS semaphore.
 * 3. Release one token and verify the high-priority waiter runs first.
 *
 * Sequence diagram:
 *   High thread          Low thread           Test thread
 *   -----------          ----------           -----------
 *   osSemaphoreWait()    osSemaphoreWait()
 *       blocked              blocked
 *                                            osSemaphoreRelease()
 *   wakes first
 *                                            osSemaphoreRelease()
 *                        wakes second
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Semaphore used to block both priority test threads. */
    osSemaphoreId sem;
    /* Number of threads that reached the blocking wait point. */
    volatile uint32_t readyCount;
    /* First observed execution order marker: 1 for high, 2 for low. */
    volatile uint32_t firstOrder;
    /* Second observed execution order marker: 1 for high, 2 for low. */
    volatile uint32_t secondOrder;
} CmsisV1PriorityE2ECtx;

static int CmsisV1PriorityE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] prioE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1PriorityRecord(CmsisV1PriorityE2ECtx *ctx, uint32_t marker)
{
    if (ctx->firstOrder == 0) {
        ctx->firstOrder = marker;
    } else if (ctx->secondOrder == 0) {
        ctx->secondOrder = marker;
    }
}

static void CmsisV1HighPriorityTask(const void *argument)
{
    CmsisV1PriorityE2ECtx *ctx = (CmsisV1PriorityE2ECtx *)argument;

    ctx->readyCount++;
    if (osSemaphoreWait(ctx->sem, osWaitForever) >= 0) {
        CmsisV1PriorityRecord(ctx, 1);
    }
}

static void CmsisV1LowPriorityTask(const void *argument)
{
    CmsisV1PriorityE2ECtx *ctx = (CmsisV1PriorityE2ECtx *)argument;

    ctx->readyCount++;
    if (osSemaphoreWait(ctx->sem, osWaitForever) >= 0) {
        CmsisV1PriorityRecord(ctx, 2);
    }
}

osSemaphoreDef(CmsisV1PrioritySem);
osThreadDef(CmsisV1HighPriorityTask, osPriorityHigh, 1, 0x1000);
osThreadDef(CmsisV1LowPriorityTask, osPriorityLow, 1, 0x1000);

int CmsisV1PriorityE2ETest(void)
{
    CmsisV1PriorityE2ECtx ctx = {0};
    osThreadId highTid;
    osThreadId lowTid;
    uint32_t wait;

    ctx.sem = osSemaphoreCreate(osSemaphore(CmsisV1PrioritySem), 0);
    if (ctx.sem == NULL) {
        return CmsisV1PriorityE2EFail("sem", 0);
    }
    lowTid = osThreadCreate(osThread(CmsisV1LowPriorityTask), &ctx);
    highTid = osThreadCreate(osThread(CmsisV1HighPriorityTask), &ctx);
    if ((highTid == NULL) || (lowTid == NULL)) {
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisV1PriorityE2EFail("thread", 0);
    }
    for (wait = 0; wait < 50 && ctx.readyCount < 2; wait++) {
        (void)osDelay(1);
    }
    if (ctx.readyCount < 2) {
        (void)osThreadTerminate(highTid);
        (void)osThreadTerminate(lowTid);
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisV1PriorityE2EFail("ready", ctx.readyCount);
    }
    (void)osSemaphoreRelease(ctx.sem);
    for (wait = 0; wait < 50 && ctx.firstOrder == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osSemaphoreRelease(ctx.sem);
    for (wait = 0; wait < 50 && ctx.secondOrder == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osThreadTerminate(highTid);
    (void)osThreadTerminate(lowTid);
    (void)osSemaphoreDelete(ctx.sem);
    if (ctx.firstOrder != 1) {
        return CmsisV1PriorityE2EFail("first", ctx.firstOrder);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] prioE2E\n");
    return 0;
}
