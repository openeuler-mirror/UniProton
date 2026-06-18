/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 timer end-to-end test.
 *
 * Test method:
 * 1. Start a waiter thread that blocks in osSignalWait().
 * 2. Create a one-shot CMSIS v1 timer whose callback signals that waiter.
 * 3. Start the timer and verify the waiter wakes only through the callback.
 *
 * Sequence diagram:
 *   Waiter thread                 Timer callback
 *   -------------                 --------------
 *   osSignalWait(0x1)
 *       -> blocked
 *                                 osSignalSet(waiter, 0x1)
 *   wakes and records signal
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Waiter thread id used by the timer callback as osSignalSet target. */
    osThreadId waiterTid;
    /* CMSIS v1 timer object under test. */
    osTimerId timer;
    /* Set when the waiter starts blocking on a signal. */
    volatile uint32_t waiterStarted;
    /* Set when the timer callback runs. */
    volatile uint32_t callbackRan;
    /* Signal bits returned from the waiter. */
    volatile uint32_t signals;
} CmsisV1TimerE2ECtx;

static CmsisV1TimerE2ECtx *g_v1TimerE2ECtx;

static int CmsisV1TimerE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] timerE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1TimerE2ECb(const void *argument)
{
    CmsisV1TimerE2ECtx *ctx = (CmsisV1TimerE2ECtx *)argument;

    if (ctx == NULL) {
        ctx = g_v1TimerE2ECtx;
    }
    if ((ctx != NULL) && (ctx->waiterTid != NULL)) {
        ctx->callbackRan = 1;
        (void)osSignalSet(ctx->waiterTid, 0x1);
    }
}

static void CmsisV1TimerWaiter(const void *argument)
{
    CmsisV1TimerE2ECtx *ctx = (CmsisV1TimerE2ECtx *)argument;
    osEvent event;

    ctx->waiterTid = osThreadGetId();
    ctx->waiterStarted = 1;
    event = osSignalWait(0x1, osWaitForever);
    if (event.status == osEventSignal) {
        ctx->signals = (uint32_t)event.value.signals;
    }
}

osTimerDef(CmsisV1TimerE2E, CmsisV1TimerE2ECb);
osThreadDef(CmsisV1TimerWaiter, osPriorityNormal, 1, 0x1000);

int CmsisV1TimerE2ETest(void)
{
    CmsisV1TimerE2ECtx ctx = {0};
    osThreadId waiterTid;
    uint32_t wait;

    g_v1TimerE2ECtx = &ctx;
    waiterTid = osThreadCreate(osThread(CmsisV1TimerWaiter), &ctx);
    if (waiterTid == NULL) {
        g_v1TimerE2ECtx = NULL;
        return CmsisV1TimerE2EFail("thread", 0);
    }
    for (wait = 0; wait < 50 && ctx.waiterStarted == 0; wait++) {
        (void)osDelay(1);
    }
    ctx.timer = osTimerCreate(osTimer(CmsisV1TimerE2E), osTimerOnce, &ctx);
    if (ctx.timer == NULL) {
        (void)osThreadTerminate(waiterTid);
        g_v1TimerE2ECtx = NULL;
        return CmsisV1TimerE2EFail("create", 0);
    }
    if (osTimerStart(ctx.timer, 20) != osOK) {
        (void)osTimerDelete(ctx.timer);
        (void)osThreadTerminate(waiterTid);
        g_v1TimerE2ECtx = NULL;
        return CmsisV1TimerE2EFail("start", 0);
    }
    for (wait = 0; wait < 100 && ctx.signals == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osTimerDelete(ctx.timer);
    (void)osThreadTerminate(waiterTid);
    g_v1TimerE2ECtx = NULL;
    if ((ctx.callbackRan != 1) || ((ctx.signals & 0x1U) == 0)) {
        return CmsisV1TimerE2EFail("callback", ctx.signals);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] timerE2E\n");
    return 0;
}
