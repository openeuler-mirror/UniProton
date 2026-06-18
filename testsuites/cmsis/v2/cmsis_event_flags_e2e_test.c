/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS event flags end-to-end test.
 *
 * Test method:
 * 1. Create a CMSIS event-flags object and two CMSIS threads.
 * 2. The waiter thread calls osEventFlagsWait() for flags 0x1 and 0x2 with
 *    osFlagsWaitAll and osWaitForever, so it must remain blocked until both
 *    flags are set.
 * 3. The setter thread sets 0x1, delays, then sets 0x2.
 * 4. The test passes only if the waiter completes after both flags are set and
 *    the returned flag value contains both bits.
 *
 * Sequence diagram:
 *   Waiter                          Setter
 *   ------                          ------
 *   osEventFlagsWait(0x3, ALL)
 *       -> blocked
 *                                   osEventFlagsSet(0x1)
 *                                   delay
 *                                   osEventFlagsSet(0x2)
 *   wakes with flags containing 0x3
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS event-flags object shared by setter and waiter threads. */
    osEventFlagsId_t ef;
    /* Set by the setter after it has set both event bits. */
    volatile uint32_t setterDone;
    /* Set by the waiter after osEventFlagsWait() returns. */
    volatile uint32_t waiterDone;
    /* Raw return value from osEventFlagsWait(); must contain both 0x1 and 0x2. */
    volatile uint32_t waitResult;
} EventFlagsE2ECtx;

static int CmsisEventFlagsE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] efE2E %s=0x%x\n", detail, value);
    return 1;
}

static void EventSetterTask(void *arg)
{
    EventFlagsE2ECtx *ctx = (EventFlagsE2ECtx *)arg;

    (void)osDelay(2);
    (void)osEventFlagsSet(ctx->ef, 0x1);
    (void)osDelay(1);
    (void)osEventFlagsSet(ctx->ef, 0x2);
    ctx->setterDone = 1;
}

static void EventWaiterTask(void *arg)
{
    EventFlagsE2ECtx *ctx = (EventFlagsE2ECtx *)arg;

    ctx->waitResult = osEventFlagsWait(ctx->ef, 0x3, osFlagsWaitAll, osWaitForever);
    ctx->waiterDone = 1;
}

int CmsisEventFlagsE2ETest(void)
{
    EventFlagsE2ECtx ctx = {0};
    osThreadAttr_t setterAttr = {0};
    osThreadAttr_t waiterAttr = {0};
    osThreadId_t setterTid;
    osThreadId_t waiterTid;
    uint32_t wait;

    ctx.ef = osEventFlagsNew(NULL);
    if (ctx.ef == NULL) {
        return CmsisEventFlagsE2EFail("new", 0);
    }

    setterAttr.name = "eventSet";
    setterAttr.priority = osPriorityNormal;
    setterAttr.stack_size = 0x1000;
    setterTid = osThreadNew(EventSetterTask, &ctx, &setterAttr);

    waiterAttr.name = "eventWait";
    waiterAttr.priority = osPriorityNormal;
    waiterAttr.stack_size = 0x1000;
    waiterTid = osThreadNew(EventWaiterTask, &ctx, &waiterAttr);

    for (wait = 0; wait < 50 && (ctx.setterDone == 0 || ctx.waiterDone == 0); wait++) {
        (void)osDelay(1);
    }

    (void)osThreadTerminate(setterTid);
    (void)osThreadTerminate(waiterTid);
    (void)osEventFlagsDelete(ctx.ef);

    if (ctx.setterDone != 1) {
        return CmsisEventFlagsE2EFail("setter", 0);
    }
    if (ctx.waiterDone != 1) {
        return CmsisEventFlagsE2EFail("waiter", 0);
    }
    if ((ctx.waitResult & 0x3) != 0x3) {
        return CmsisEventFlagsE2EFail("result", ctx.waitResult);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] efE2E\n");
    return 0;
}
