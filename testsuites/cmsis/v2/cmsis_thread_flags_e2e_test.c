/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS thread flags end-to-end test.
 *
 * Test method:
 * 1. Start a receiver thread that calls osThreadFlagsWait() for 0x1 and 0x4
 *    with osFlagsWaitAll and osWaitForever.
 * 2. The main test thread sets 0x1 first, delays, then sets 0x4.
 * 3. The receiver must not complete after only 0x1; it may complete only after
 *    both bits are present.
 * 4. The test passes if the receiver wakes and the returned flag value contains
 *    both requested bits.
 *
 * Sequence diagram:
 *   Receiver                         Main test thread
 *   --------                         ----------------
 *   osThreadFlagsWait(0x5, ALL)
 *       -> blocked
 *                                    osThreadFlagsSet(receiver, 0x1)
 *                                    delay; receiver still blocked
 *                                    osThreadFlagsSet(receiver, 0x4)
 *   wakes with flags containing 0x5
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* Raw return value from osThreadFlagsWait(); must contain both 0x1 and 0x4. */
    volatile uint32_t flagsReceived;
    /* Set by receiver after osThreadFlagsWait() returns. */
    volatile uint32_t taskDone;
} ThreadFlagsE2ECtx;

static int CmsisThreadFlagsE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] tfE2E %s=0x%x\n", detail, value);
    return 1;
}

static void FlagReceiverTask(void *arg)
{
    ThreadFlagsE2ECtx *ctx = (ThreadFlagsE2ECtx *)arg;

    ctx->flagsReceived = osThreadFlagsWait(0x5, osFlagsWaitAll, osWaitForever);
    ctx->taskDone = 1;
}

int CmsisThreadFlagsE2ETest(void)
{
    ThreadFlagsE2ECtx ctx = {0};
    osThreadAttr_t attr = {0};
    osThreadId_t tid;
    uint32_t wait;

    attr.name = "flagRecv";
    attr.priority = osPriorityNormal;
    attr.stack_size = 0x1000;
    tid = osThreadNew(FlagReceiverTask, &ctx, &attr);
    if (tid == NULL) {
        return CmsisThreadFlagsE2EFail("new", 0);
    }

    (void)osDelay(2);
    (void)osThreadFlagsSet(tid, 0x1);
    (void)osDelay(1);
    (void)osThreadFlagsSet(tid, 0x4);

    for (wait = 0; wait < 30 && ctx.taskDone == 0; wait++) {
        (void)osDelay(1);
    }

    (void)osThreadTerminate(tid);

    if (ctx.taskDone != 1) {
        return CmsisThreadFlagsE2EFail("notDone", 0);
    }
    if ((ctx.flagsReceived & 0x5) != 0x5) {
        return CmsisThreadFlagsE2EFail("flags", ctx.flagsReceived);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] tfE2E\n");
    return 0;
}
