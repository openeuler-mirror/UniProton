/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS thread suspend/resume end-to-end test.
 *
 * Test method:
 * 1. Start a CMSIS worker thread that increments a counter and calls osDelay().
 * 2. After the counter changes, suspend the worker with osThreadSuspend().
 * 3. Delay in the controller thread and verify the worker counter no longer
 *    changes while suspended.
 * 4. Resume the worker with osThreadResume() and verify the counter changes
 *    again.
 * 5. The test passes only if execution stops during suspend and continues after
 *    resume, all through CMSIS thread APIs.
 *
 * Sequence diagram:
 *   Worker thread                    Controller thread
 *   -------------                    -----------------
 *   ticks++ loop
 *                                    observe ticks changed
 *                                    osThreadSuspend(worker)
 *   stopped; ticks unchanged
 *                                    osThreadResume(worker)
 *   ticks++ again
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Incremented by the worker thread while it is running. */
    volatile uint32_t ticks;
    /* Set by the controller to ask the worker loop to exit. */
    volatile uint32_t stop;
} SuspendResumeE2ECtx;

static int CmsisSuspendResumeE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] suspendResumeE2E %s=0x%x\n", detail, value);
    return 1;
}

static void SuspendResumeTask(void *arg)
{
    SuspendResumeE2ECtx *ctx = (SuspendResumeE2ECtx *)arg;

    while (ctx->stop == 0) {
        ctx->ticks++;
        (void)osDelay(1);
    }
}

int CmsisSuspendResumeE2ETest(void)
{
    SuspendResumeE2ECtx ctx = {0};
    osThreadAttr_t attr = {0};
    osThreadId_t tid;
    uint32_t beforeSuspend;
    uint32_t duringSuspend;
    uint32_t wait;

    attr.name = "suspendResume";
    attr.priority = osPriorityNormal;
    attr.stack_size = 0x1000;
    tid = osThreadNew(SuspendResumeTask, &ctx, &attr);
    if (tid == NULL) {
        return CmsisSuspendResumeE2EFail("new", 0);
    }

    for (wait = 0; wait < 20 && ctx.ticks < 2; wait++) {
        (void)osDelay(1);
    }
    if (ctx.ticks < 2) {
        (void)osThreadTerminate(tid);
        return CmsisSuspendResumeE2EFail("notRun", ctx.ticks);
    }
    if (osThreadSuspend(tid) != osOK) {
        (void)osThreadTerminate(tid);
        return CmsisSuspendResumeE2EFail("suspend", 0);
    }

    beforeSuspend = ctx.ticks;
    (void)osDelay(5);
    duringSuspend = ctx.ticks;
    if (duringSuspend != beforeSuspend) {
        (void)osThreadTerminate(tid);
        return CmsisSuspendResumeE2EFail("ranSuspended", duringSuspend - beforeSuspend);
    }
    if (osThreadResume(tid) != osOK) {
        (void)osThreadTerminate(tid);
        return CmsisSuspendResumeE2EFail("resume", 0);
    }
    for (wait = 0; wait < 20 && ctx.ticks == duringSuspend; wait++) {
        (void)osDelay(1);
    }
    ctx.stop = 1;
    (void)osDelay(1);
    (void)osThreadTerminate(tid);

    if (ctx.ticks == duringSuspend) {
        return CmsisSuspendResumeE2EFail("notResumed", ctx.ticks);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] suspendResumeE2E\n");
    return 0;
}
