/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS mutex end-to-end test.
 *
 * Test method:
 * 1. Create one CMSIS mutex and start two CMSIS threads.
 * 2. Thread 1 acquires the mutex and delays while holding it.
 * 3. Thread 2 verifies osMutexGetOwner() reports Thread 1, then calls
 *    osMutexAcquire(timeout=0). This must fail with osErrorTimeout because
 *    Thread 1 still owns the mutex.
 * 4. Thread 2 then waits with osWaitForever and must acquire the mutex only
 *    after Thread 1 releases it.
 * 5. The test passes only if no critical-section overlap is observed, owner
 *    query is correct, non-blocking acquire fails while locked, and blocking
 *    acquire succeeds after release.
 *
 * Sequence diagram:
 *   Thread 1                         Thread 2
 *   --------                         --------
 *   osMutexAcquire() -> osOK
 *   hold mutex and delay
 *                                    osMutexGetOwner() == Thread 1
 *                                    osMutexAcquire(timeout=0)
 *                                        -> osErrorTimeout
 *                                    osMutexAcquire(osWaitForever)
 *                                        -> blocked
 *   osMutexRelease()
 *                                    wakes, enters critical section
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS mutex object used to protect the tested critical section. */
    osMutexId_t mutex;
    /* CMSIS thread id of Thread 1, used to verify osMutexGetOwner(). */
    osThreadId_t task1Id;
    /* Thread 1 critical-section state: 0=not entered, 1=inside, 2=left. */
    volatile uint32_t task1In;
    /* Thread 2 critical-section state: 0=not entered, 1=inside, 2=left. */
    volatile uint32_t task2In;
    /* Set to 1 if both threads are observed in the critical section together. */
    volatile uint32_t conflict;
    /* Set to 1 when osMutexGetOwner() reports Thread 1 while Thread 1 holds the lock. */
    volatile uint32_t ownerMatched;
    /* Return status of Thread 2 non-blocking acquire while Thread 1 owns the mutex. */
    volatile osStatus_t blockedAcquireStatus;
} MutexE2ECtx;

static int CmsisMutexE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] mutexE2E %s=0x%x\n", detail, value);
    return 1;
}

static void MutexTask1(void *arg)
{
    MutexE2ECtx *ctx = (MutexE2ECtx *)arg;

    if (osMutexAcquire(ctx->mutex, osWaitForever) == osOK) {
        ctx->task1In = 1;
        (void)osDelay(3);
        if (ctx->task2In != 0) {
            ctx->conflict = 1;
        }
        ctx->task1In = 2;
        (void)osMutexRelease(ctx->mutex);
    }
}

static void MutexTask2(void *arg)
{
    MutexE2ECtx *ctx = (MutexE2ECtx *)arg;

    while (ctx->task1In != 1) {
        (void)osDelay(1);
    }
    ctx->ownerMatched = (osMutexGetOwner(ctx->mutex) == ctx->task1Id) ? 1 : 0;
    ctx->blockedAcquireStatus = osMutexAcquire(ctx->mutex, 0);
    if (osMutexAcquire(ctx->mutex, osWaitForever) == osOK) {
        ctx->task2In = 1;
        (void)osDelay(2);
        if (ctx->task1In == 1) {
            ctx->conflict = 1;
        }
        ctx->task2In = 2;
        (void)osMutexRelease(ctx->mutex);
    }
}

int CmsisMutexE2ETest(void)
{
    MutexE2ECtx ctx = {0};
    osThreadAttr_t attr1 = {0};
    osThreadAttr_t attr2 = {0};
    osThreadId_t tid1;
    osThreadId_t tid2;
    uint32_t wait;

    ctx.mutex = osMutexNew(NULL);
    if (ctx.mutex == NULL) {
        return CmsisMutexE2EFail("new", 0);
    }

    attr1.name = "mutex1";
    attr1.priority = osPriorityNormal;
    attr1.stack_size = 0x1000;
    tid1 = osThreadNew(MutexTask1, &ctx, &attr1);
    if (tid1 == NULL) {
        (void)osMutexDelete(ctx.mutex);
        return CmsisMutexE2EFail("task1New", 0);
    }
    ctx.task1Id = tid1;

    attr2.name = "mutex2";
    attr2.priority = osPriorityNormal;
    attr2.stack_size = 0x1000;
    tid2 = osThreadNew(MutexTask2, &ctx, &attr2);
    if (tid2 == NULL) {
        (void)osThreadTerminate(tid1);
        (void)osMutexDelete(ctx.mutex);
        return CmsisMutexE2EFail("task2New", 0);
    }

    for (wait = 0; wait < 50 && (ctx.task1In != 2 || ctx.task2In != 2); wait++) {
        (void)osDelay(1);
    }

    (void)osThreadTerminate(tid1);
    (void)osThreadTerminate(tid2);
    (void)osMutexDelete(ctx.mutex);

    if (ctx.conflict != 0) {
        return CmsisMutexE2EFail("conflict", ctx.conflict);
    }
    if (ctx.ownerMatched != 1) {
        return CmsisMutexE2EFail("owner", ctx.ownerMatched);
    }
    if (ctx.blockedAcquireStatus != osErrorTimeout) {
        return CmsisMutexE2EFail("blockedAcquire", (uint32_t)ctx.blockedAcquireStatus);
    }
    if (ctx.task1In != 2 || ctx.task2In != 2) {
        return CmsisMutexE2EFail("incomplete", ctx.task1In * 10 + ctx.task2In);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] mutexE2E\n");
    return 0;
}
