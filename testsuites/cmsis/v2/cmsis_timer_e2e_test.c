/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS timer end-to-end test.
 *
 * Test method:
 * 1. Start a waiter thread that blocks in osThreadFlagsWait().
 * 2. Create a one-shot CMSIS timer whose callback sets the waiter's thread flag.
 * 3. Start the timer and wait for the waiter thread to be woken.
 * 4. The test passes only if the timer callback runs and its interaction with
 *    the waiting thread wakes that thread through CMSIS thread flags.
 *
 * Sequence diagram:
 *   Waiter thread                    Timer callback
 *   -------------                    --------------
 *   osThreadFlagsWait(0x1)
 *       -> blocked
 *                                    timer expires
 *                                    osThreadFlagsSet(waiter, 0x1)
 *   wakes with flag 0x1
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS one-shot timer object under test. */
    osTimerId_t timer;
    /* CMSIS thread id of the waiter woken by the timer callback. */
    osThreadId_t waitTaskId;
    /* Set by timer callback when the timer expires. */
    volatile uint32_t timerFired;
    /* Raw return value from waiter's osThreadFlagsWait(). */
    volatile uint32_t taskWoken;
} TimerE2ECtx;

static int CmsisTimerE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] timerE2E %s=0x%x\n", detail, value);
    return 1;
}

static void TimerCallback(void *arg)
{
    TimerE2ECtx *ctx = (TimerE2ECtx *)arg;

    ctx->timerFired = 1;
    if (ctx->waitTaskId != NULL) {
        (void)osThreadFlagsSet(ctx->waitTaskId, 0x1);
    }
}

static void TimerWaitTask(void *arg)
{
    TimerE2ECtx *ctx = (TimerE2ECtx *)arg;

    ctx->taskWoken = osThreadFlagsWait(0x1, osFlagsWaitAny, osWaitForever);
}

int CmsisTimerE2ETest(void)
{
    TimerE2ECtx ctx = {0};
    osThreadAttr_t attr = {0};
    osThreadId_t tid;
    uint32_t wait;

    attr.name = "tmrWait";
    attr.priority = osPriorityNormal;
    attr.stack_size = 0x1000;
    tid = osThreadNew(TimerWaitTask, &ctx, &attr);
    if (tid == NULL) {
        return CmsisTimerE2EFail("newTask", 0);
    }

    ctx.waitTaskId = tid;
    ctx.timer = osTimerNew(TimerCallback, osTimerOnce, &ctx, NULL);
    if (ctx.timer == NULL) {
        (void)osThreadTerminate(tid);
        return CmsisTimerE2EFail("newTimer", 0);
    }

    (void)osTimerStart(ctx.timer, 5);

    for (wait = 0; wait < 50 && ctx.taskWoken == 0; wait++) {
        (void)osDelay(1);
    }

    (void)osTimerDelete(ctx.timer);
    (void)osThreadTerminate(tid);

    if (ctx.timerFired != 1) {
        return CmsisTimerE2EFail("notFired", 0);
    }
    if ((ctx.taskWoken & 0x1) != 0x1) {
        return CmsisTimerE2EFail("notWoken", ctx.taskWoken);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] timerE2E\n");
    return 0;
}
