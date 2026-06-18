/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 semaphore end-to-end test.
 *
 * Test method:
 * 1. Create one CMSIS v1 semaphore with initial count 0.
 * 2. Verify osSemaphoreWait(timeout=0) fails while no token is available.
 * 3. Start one waiter thread. It blocks in osSemaphoreWait(osWaitForever).
 * 4. The test thread releases the semaphore and the waiter must wake.
 *
 * Sequence diagram:
 *   Test thread                  Waiter thread
 *   -----------                  -------------
 *   osSemaphoreWait(0) -> fail
 *                                osSemaphoreWait(osWaitForever)
 *                                    -> blocked
 *   osSemaphoreRelease()
 *                                wakes and returns >= 0
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS v1 semaphore shared by the test thread and waiter thread. */
    osSemaphoreId sem;
    /* Set by the waiter immediately before blocking on the semaphore. */
    volatile uint32_t waiterStarted;
    /* Set by the waiter after osSemaphoreWait() succeeds. */
    volatile uint32_t waiterWoke;
    /* Raw osSemaphoreWait(osWaitForever) return value from the waiter. */
    volatile int32_t waiterStatus;
    /* Raw osSemaphoreWait(timeout=0) return value observed while empty. */
    volatile int32_t emptyTryStatus;
} CmsisV1SemE2ECtx;

static int CmsisV1SemE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] semE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1SemWaiter(const void *argument)
{
    CmsisV1SemE2ECtx *ctx = (CmsisV1SemE2ECtx *)argument;

    ctx->waiterStarted = 1;
    ctx->waiterStatus = osSemaphoreWait(ctx->sem, osWaitForever);
    if (ctx->waiterStatus >= 0) {
        ctx->waiterWoke = 1;
    }
}

osSemaphoreDef(CmsisV1SemE2E);
osThreadDef(CmsisV1SemWaiter, osPriorityNormal, 1, 0x1000);

int CmsisV1SemaphoreE2ETest(void)
{
    CmsisV1SemE2ECtx ctx = {0};
    osThreadId waiterTid;
    uint32_t wait;

    ctx.sem = osSemaphoreCreate(osSemaphore(CmsisV1SemE2E), 0);
    if (ctx.sem == NULL) {
        return CmsisV1SemE2EFail("create", 0);
    }
    ctx.emptyTryStatus = osSemaphoreWait(ctx.sem, 0);
    if (ctx.emptyTryStatus >= 0) {
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisV1SemE2EFail("emptyWait", (uint32_t)ctx.emptyTryStatus);
    }
    waiterTid = osThreadCreate(osThread(CmsisV1SemWaiter), &ctx);
    if (waiterTid == NULL) {
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisV1SemE2EFail("thread", 0);
    }
    for (wait = 0; wait < 50 && ctx.waiterStarted == 0; wait++) {
        (void)osDelay(1);
    }
    if (ctx.waiterStarted == 0) {
        (void)osThreadTerminate(waiterTid);
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisV1SemE2EFail("notStarted", 0);
    }
    if (osSemaphoreRelease(ctx.sem) != osOK) {
        (void)osThreadTerminate(waiterTid);
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisV1SemE2EFail("release", 0);
    }
    for (wait = 0; wait < 50 && ctx.waiterWoke == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osThreadTerminate(waiterTid);
    (void)osSemaphoreDelete(ctx.sem);
    if ((ctx.waiterWoke != 1) || (ctx.waiterStatus < 0)) {
        return CmsisV1SemE2EFail("notWoken", (uint32_t)ctx.waiterStatus);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] semE2E\n");
    return 0;
}
