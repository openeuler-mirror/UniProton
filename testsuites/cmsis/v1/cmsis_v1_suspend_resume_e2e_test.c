/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 self-suspend/resume end-to-end test.
 *
 * Test method:
 * 1. Start one worker thread.
 * 2. The worker marks itself started, then calls osThreadSelfSuspend().
 * 3. The test thread resumes it with osThreadResume(worker).
 * 4. The worker must continue after the suspend point and set resumed=1.
 *
 * Sequence diagram:
 *   Worker thread                         Test thread
 *   -------------                         -----------
 *   started = 1
 *   osThreadSelfSuspend() -> suspended
 *                                         osThreadResume(worker)
 *   resumed = 1
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Set by the worker before self-suspending. */
    volatile uint32_t started;
    /* Set by the worker after the test thread resumes it. */
    volatile uint32_t resumed;
    /* Raw osThreadSelfSuspend() return status observed by the worker. */
    volatile osStatus suspendStatus;
} CmsisV1SuspendResumeE2ECtx;

static int CmsisV1SuspendResumeE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] suspendResumeE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1SuspendResumeWorker(const void *argument)
{
    CmsisV1SuspendResumeE2ECtx *ctx = (CmsisV1SuspendResumeE2ECtx *)argument;

    ctx->started = 1;
    ctx->suspendStatus = osThreadSelfSuspend();
    ctx->resumed = 1;
}

osThreadDef(CmsisV1SuspendResumeWorker, osPriorityNormal, 1, 0x1000);

int CmsisV1SuspendResumeE2ETest(void)
{
    CmsisV1SuspendResumeE2ECtx ctx = {0};
    osThreadId workerTid;
    uint32_t wait;

    workerTid = osThreadCreate(osThread(CmsisV1SuspendResumeWorker), &ctx);
    if (workerTid == NULL) {
        return CmsisV1SuspendResumeE2EFail("thread", 0);
    }
    for (wait = 0; wait < 50 && ctx.started == 0; wait++) {
        (void)osDelay(1);
    }
    if (ctx.started == 0) {
        (void)osThreadTerminate(workerTid);
        return CmsisV1SuspendResumeE2EFail("notStarted", 0);
    }
    (void)osDelay(2);
    if (ctx.resumed != 0) {
        (void)osThreadTerminate(workerTid);
        return CmsisV1SuspendResumeE2EFail("notSuspended", ctx.resumed);
    }
    if (osThreadResume(workerTid) != osOK) {
        (void)osThreadTerminate(workerTid);
        return CmsisV1SuspendResumeE2EFail("resume", 0);
    }
    for (wait = 0; wait < 50 && ctx.resumed == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osThreadTerminate(workerTid);
    if (ctx.resumed != 1) {
        return CmsisV1SuspendResumeE2EFail("notResumed", ctx.resumed);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] suspendResumeE2E\n");
    return 0;
}
