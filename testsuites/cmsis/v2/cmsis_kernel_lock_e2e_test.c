/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS kernel-lock end-to-end test.
 *
 * Test method:
 * 1. Lock the kernel scheduler with osKernelLock().
 * 2. Create a worker thread while the scheduler is locked.
 * 3. Delay in the controller thread and verify the worker did not run while the
 *    kernel was locked.
 * 4. Unlock the scheduler with osKernelUnlock() and verify the worker starts.
 * 5. The test passes only if thread execution is suppressed during the lock and
 *    resumes after unlock, using only CMSIS kernel/thread APIs.
 *
 * Sequence diagram:
 *   Controller thread                Worker thread
 *   -----------------                -------------
 *   osKernelLock()
 *   osThreadNew(worker)
 *   delay while locked
 *   verify ran == 0                  not scheduled yet
 *   osKernelUnlock()
 *                                    starts and sets ran = 1
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Set by the worker thread when it actually starts executing. */
    volatile uint32_t ran;
    /* Set by the controller to ask the worker loop to exit. */
    volatile uint32_t stop;
} KernelLockE2ECtx;

static int CmsisKernelLockE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] lockE2E %s=0x%x\n", detail, value);
    return 1;
}

static void KernelLockedTask(void *argument)
{
    KernelLockE2ECtx *ctx = (KernelLockE2ECtx *)argument;

    ctx->ran = 1;
    while (ctx->stop == 0) {
        (void)osDelay(1);
    }
}

int CmsisKernelLockE2ETest(void)
{
    osThreadAttr_t attr = {0};
    osThreadId_t tid;
    KernelLockE2ECtx ctx = {0};
    uint32_t wait;
    int32_t lockState;

    lockState = osKernelLock();
    if (lockState < 0) {
        return CmsisKernelLockE2EFail("lock", (uint32_t)lockState);
    }

    attr.name = "lockedTask";
    attr.priority = osPriorityNormal;
    attr.stack_size = 0x1000;
    tid = osThreadNew(KernelLockedTask, &ctx, &attr);
    if (tid == NULL) {
        (void)osKernelUnlock();
        return CmsisKernelLockE2EFail("new", 0);
    }

    for (wait = 0; wait < 10; wait++) {
        (void)osDelay(1);
    }

    if (ctx.ran != 0) {
        (void)osThreadTerminate(tid);
        (void)osKernelUnlock();
        return CmsisKernelLockE2EFail("ranWhileLocked", ctx.ran);
    }

    (void)osKernelUnlock();

    for (wait = 0; wait < 20 && ctx.ran == 0; wait++) {
        (void)osDelay(1);
    }

    ctx.stop = 1;
    (void)osThreadTerminate(tid);

    if (ctx.ran != 1) {
        return CmsisKernelLockE2EFail("notRanAfterUnlock", 0);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] lockE2E\n");
    return 0;
}
