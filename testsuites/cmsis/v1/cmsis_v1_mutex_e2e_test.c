/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 mutex end-to-end test.
 *
 * Test method:
 * 1. Thread 1 acquires a CMSIS v1 mutex and holds it for several ticks.
 * 2. Thread 2 verifies osMutexWait(timeout=0) fails while Thread 1 owns it.
 * 3. Thread 2 waits forever and must acquire the mutex after Thread 1 releases it.
 *
 * Sequence diagram:
 *   Thread 1                         Thread 2
 *   --------                         --------
 *   osMutexWait() -> osOK
 *                                    osMutexWait(0) -> timeout/resource error
 *                                    osMutexWait(osWaitForever) -> blocked
 *   osMutexRelease()
 *                                    wakes, acquires, releases
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS v1 mutex contended by both worker threads. */
    osMutexId mutex;
    /* Set when Thread 1 owns the mutex. */
    volatile uint32_t holderLocked;
    /* Set when Thread 1 releases the mutex. */
    volatile uint32_t holderReleased;
    /* Set when Thread 2 acquires the mutex after blocking. */
    volatile uint32_t waiterAcquired;
    /* Thread 2 non-blocking wait result while Thread 1 owns the mutex. */
    volatile osStatus tryStatus;
} CmsisV1MutexE2ECtx;

static int CmsisV1MutexE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] mutexE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1MutexHolder(const void *argument)
{
    CmsisV1MutexE2ECtx *ctx = (CmsisV1MutexE2ECtx *)argument;

    if (osMutexWait(ctx->mutex, osWaitForever) == osOK) {
        ctx->holderLocked = 1;
        (void)osDelay(10);
        (void)osMutexRelease(ctx->mutex);
        ctx->holderReleased = 1;
    }
}

static void CmsisV1MutexWaiter(const void *argument)
{
    CmsisV1MutexE2ECtx *ctx = (CmsisV1MutexE2ECtx *)argument;

    while (ctx->holderLocked == 0) {
        (void)osDelay(1);
    }
    ctx->tryStatus = osMutexWait(ctx->mutex, 0);
    if (osMutexWait(ctx->mutex, osWaitForever) == osOK) {
        ctx->waiterAcquired = 1;
        (void)osMutexRelease(ctx->mutex);
    }
}

osMutexDef(CmsisV1MutexE2E);
osThreadDef(CmsisV1MutexHolder, osPriorityNormal, 1, 0x1000);
osThreadDef(CmsisV1MutexWaiter, osPriorityNormal, 1, 0x1000);

int CmsisV1MutexE2ETest(void)
{
    CmsisV1MutexE2ECtx ctx = {0};
    osThreadId holderTid;
    osThreadId waiterTid;
    uint32_t wait;

    ctx.mutex = osMutexCreate(osMutex(CmsisV1MutexE2E));
    if (ctx.mutex == NULL) {
        return CmsisV1MutexE2EFail("create", 0);
    }
    holderTid = osThreadCreate(osThread(CmsisV1MutexHolder), &ctx);
    waiterTid = osThreadCreate(osThread(CmsisV1MutexWaiter), &ctx);
    if ((holderTid == NULL) || (waiterTid == NULL)) {
        (void)osMutexDelete(ctx.mutex);
        return CmsisV1MutexE2EFail("thread", 0);
    }
    for (wait = 0; wait < 100 && ctx.waiterAcquired == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osThreadTerminate(holderTid);
    (void)osThreadTerminate(waiterTid);
    (void)osMutexDelete(ctx.mutex);
    if (ctx.holderLocked != 1 || ctx.holderReleased != 1 || ctx.waiterAcquired != 1) {
        return CmsisV1MutexE2EFail("flow", ctx.waiterAcquired);
    }
    if (ctx.tryStatus == osOK) {
        return CmsisV1MutexE2EFail("tryWhileLocked", (uint32_t)ctx.tryStatus);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] mutexE2E\n");
    return 0;
}
