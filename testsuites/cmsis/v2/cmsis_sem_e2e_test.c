/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS semaphore end-to-end test.
 *
 * Test method:
 * 1. Create one CMSIS semaphore with max_count=1 and initial_count=0.
 * 2. Verify osSemaphoreAcquire(timeout=0) fails with osErrorTimeout while no
 *    token is available.
 * 3. Start one waiter thread. It calls osSemaphoreAcquire(osWaitForever) and
 *    must become blocked on the empty semaphore.
 * 4. The test thread calls osSemaphoreRelease(). The waiter must wake up,
 *    acquire the token, and complete.
 * 5. The test passes only if the empty timeout path, blocking wait path,
 *    release path, and wake-up path are all observed.
 *
 * Sequence diagram:
 *   semaphore count = 0
 *
 *   Test thread                 Waiter thread
 *   -----------                 -------------
 *   osSemaphoreAcquire(0)
 *       -> osErrorTimeout
 *                               osSemaphoreAcquire(osWaitForever)
 *                                   -> blocked
 *   osSemaphoreRelease()
 *                               wakes and returns osOK
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS semaphore object shared by the test thread and waiter thread. */
    osSemaphoreId_t sem;
    /* Set by the waiter thread immediately before it waits on the empty semaphore. */
    volatile uint32_t waiterStarted;
    /* Set by the waiter thread after osSemaphoreAcquire() returns osOK. */
    volatile uint32_t waiterWoke;
    /* Return status of osSemaphoreAcquire(osWaitForever) in the waiter thread. */
    volatile osStatus_t waiterStatus;
    /* Return status of the test thread's non-blocking acquire while count is zero. */
    volatile osStatus_t emptyTryStatus;
} SemaphoreE2ECtx;

static int CmsisSemE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] semE2E %s=0x%x\n", detail, value);
    return 1;
}

static void SemWaiterTask(void *arg)
{
    SemaphoreE2ECtx *ctx = (SemaphoreE2ECtx *)arg;

    ctx->waiterStarted = 1;
    ctx->waiterStatus = osSemaphoreAcquire(ctx->sem, osWaitForever);
    if (ctx->waiterStatus == osOK) {
        ctx->waiterWoke = 1;
    }
}

int CmsisSemaphoreE2ETest(void)
{
    SemaphoreE2ECtx ctx = {0};
    osThreadAttr_t waiterAttr = {0};
    osThreadId_t waiterTid;
    osThreadState_t waiterState;
    uint32_t wait;
    uint32_t countAfterWake;

    ctx.sem = osSemaphoreNew(1, 0, NULL);
    if (ctx.sem == NULL) {
        return CmsisSemE2EFail("new", 0);
    }

    ctx.emptyTryStatus = osSemaphoreAcquire(ctx.sem, 0);
    if (ctx.emptyTryStatus != osErrorTimeout) {
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisSemE2EFail("emptyAcquire", (uint32_t)ctx.emptyTryStatus);
    }

    waiterAttr.name = "semWaiter";
    waiterAttr.priority = osPriorityNormal;
    waiterAttr.stack_size = 0x1000;
    waiterTid = osThreadNew(SemWaiterTask, &ctx, &waiterAttr);
    if (waiterTid == NULL) {
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisSemE2EFail("waiterNew", 0);
    }

    for (wait = 0; wait < 50 && ctx.waiterStarted != 1; wait++) {
        (void)osDelay(1);
    }
    if (ctx.waiterStarted != 1) {
        (void)osThreadTerminate(waiterTid);
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisSemE2EFail("waiterNotStart", 0);
    }
    for (wait = 0; wait < 50; wait++) {
        waiterState = osThreadGetState(waiterTid);
        if (waiterState == osThreadBlocked) {
            break;
        }
        (void)osDelay(1);
    }
    if (waiterState != osThreadBlocked) {
        (void)osThreadTerminate(waiterTid);
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisSemE2EFail("waiterNotBlocked", (uint32_t)waiterState);
    }
    if (osSemaphoreRelease(ctx.sem) != osOK) {
        (void)osThreadTerminate(waiterTid);
        (void)osSemaphoreDelete(ctx.sem);
        return CmsisSemE2EFail("release", 0);
    }

    for (wait = 0; wait < 50 && ctx.waiterWoke != 1; wait++) {
        (void)osDelay(1);
    }
    countAfterWake = osSemaphoreGetCount(ctx.sem);
    (void)osThreadTerminate(waiterTid);
    (void)osSemaphoreDelete(ctx.sem);

    if (ctx.waiterStatus != osOK) {
        return CmsisSemE2EFail("waitStatus", (uint32_t)ctx.waiterStatus);
    }
    if (ctx.waiterWoke != 1) {
        return CmsisSemE2EFail("waiterNotWoken", 0);
    }
    if (countAfterWake != 0) {
        return CmsisSemE2EFail("countAfterWake", countAfterWake);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] semE2E\n");
    return 0;
}
