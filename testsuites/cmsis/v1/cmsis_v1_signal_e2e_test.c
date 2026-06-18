/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 signal end-to-end test.
 *
 * Test method:
 * 1. Start a waiter thread that waits for signal mask 0x3.
 * 2. Set only signal bit 0x1 and verify the waiter is still blocked.
 * 3. Set signal bit 0x2. The waiter must wake with both bits observed.
 *
 * Sequence diagram:
 *   Test thread                  Waiter thread
 *   -----------                  -------------
 *                                osSignalWait(0x3, osWaitForever)
 *   osSignalSet(waiter, 0x1)         still waiting
 *   osSignalSet(waiter, 0x2)     wakes with 0x3
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Set when the waiter is about to call osSignalWait(). */
    volatile uint32_t waiterStarted;
    /* Set after osSignalWait() returns osEventSignal. */
    volatile uint32_t waiterWoke;
    /* Raw signal bits returned from osSignalWait(). */
    volatile uint32_t signals;
    /* Raw event status returned from osSignalWait(). */
    volatile osStatus status;
} CmsisV1SignalE2ECtx;

static int CmsisV1SignalE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] signalE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1SignalWaiter(const void *argument)
{
    CmsisV1SignalE2ECtx *ctx = (CmsisV1SignalE2ECtx *)argument;
    osEvent event;

    ctx->waiterStarted = 1;
    event = osSignalWait(0x3, osWaitForever);
    ctx->status = event.status;
    ctx->signals = (uint32_t)event.value.signals;
    if (event.status == osEventSignal) {
        ctx->waiterWoke = 1;
    }
}

osThreadDef(CmsisV1SignalWaiter, osPriorityNormal, 1, 0x1000);

int CmsisV1SignalE2ETest(void)
{
    CmsisV1SignalE2ECtx ctx = {0};
    osThreadId waiterTid;
    uint32_t wait;

    waiterTid = osThreadCreate(osThread(CmsisV1SignalWaiter), &ctx);
    if (waiterTid == NULL) {
        return CmsisV1SignalE2EFail("thread", 0);
    }
    for (wait = 0; wait < 50 && ctx.waiterStarted == 0; wait++) {
        (void)osDelay(1);
    }
    if (ctx.waiterStarted == 0) {
        (void)osThreadTerminate(waiterTid);
        return CmsisV1SignalE2EFail("notStarted", 0);
    }
    if (osSignalSet(waiterTid, 0x1) < 0) {
        (void)osThreadTerminate(waiterTid);
        return CmsisV1SignalE2EFail("set1", 0);
    }
    (void)osDelay(2);
    if (ctx.waiterWoke != 0) {
        (void)osThreadTerminate(waiterTid);
        return CmsisV1SignalE2EFail("earlyWake", ctx.signals);
    }
    if (osSignalSet(waiterTid, 0x2) < 0) {
        (void)osThreadTerminate(waiterTid);
        return CmsisV1SignalE2EFail("set2", 0);
    }
    for (wait = 0; wait < 50 && ctx.waiterWoke == 0; wait++) {
        (void)osDelay(1);
    }
    (void)osThreadTerminate(waiterTid);
    if ((ctx.waiterWoke != 1) || ((ctx.signals & 0x3U) != 0x3U)) {
        return CmsisV1SignalE2EFail("notWoken", ctx.signals);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] signalE2E\n");
    return 0;
}
