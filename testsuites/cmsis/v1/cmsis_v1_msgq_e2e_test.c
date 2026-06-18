/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 message queue end-to-end test.
 *
 * Test method:
 * 1. Create a CMSIS v1 message queue.
 * 2. Start a consumer thread that blocks in osMessageGet(osWaitForever).
 * 3. The test thread sends two messages with osMessagePut().
 * 4. The consumer must receive both messages in order.
 *
 * Sequence diagram:
 *   Producer/test thread           Consumer thread
 *   --------------------           ---------------
 *                                  osMessageGet(osWaitForever)
 *                                      -> blocked
 *   osMessagePut(0x11)
 *                                  wakes and records 0x11
 *   osMessagePut(0x22)
 *                                  records 0x22
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS v1 message queue shared by producer and consumer. */
    osMessageQId queue;
    /* Set by consumer before it starts the first blocking receive. */
    volatile uint32_t consumerStarted;
    /* Number of messages received by the consumer. */
    volatile uint32_t received;
    /* First message value observed by the consumer. */
    volatile uint32_t first;
    /* Second message value observed by the consumer. */
    volatile uint32_t second;
    /* Final event status from the consumer. */
    volatile osStatus status;
} CmsisV1MsgQE2ECtx;

static int CmsisV1MsgQE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] queueE2E %s=0x%x\n", detail, value);
    return 1;
}

static void CmsisV1MsgQConsumer(const void *argument)
{
    CmsisV1MsgQE2ECtx *ctx = (CmsisV1MsgQE2ECtx *)argument;
    osEvent event;

    ctx->consumerStarted = 1;
    event = osMessageGet(ctx->queue, osWaitForever);
    ctx->status = event.status;
    if (event.status == osEventMessage) {
        ctx->first = event.value.v;
        ctx->received++;
    }
    event = osMessageGet(ctx->queue, osWaitForever);
    ctx->status = event.status;
    if (event.status == osEventMessage) {
        ctx->second = event.value.v;
        ctx->received++;
    }
}

osMessageQDef(CmsisV1MsgQE2E, 2, uint32_t);
osThreadDef(CmsisV1MsgQConsumer, osPriorityNormal, 1, 0x1000);

int CmsisV1MessageQueueE2ETest(void)
{
    CmsisV1MsgQE2ECtx ctx = {0};
    osThreadId consumerTid;
    uint32_t wait;

    ctx.queue = osMessageCreate(osMessageQ(CmsisV1MsgQE2E), NULL);
    if (ctx.queue == NULL) {
        return CmsisV1MsgQE2EFail("create", 0);
    }
    consumerTid = osThreadCreate(osThread(CmsisV1MsgQConsumer), &ctx);
    if (consumerTid == NULL) {
        (void)osMessageDelete(ctx.queue);
        return CmsisV1MsgQE2EFail("thread", 0);
    }
    for (wait = 0; wait < 50 && ctx.consumerStarted == 0; wait++) {
        (void)osDelay(1);
    }
    if (ctx.consumerStarted == 0) {
        (void)osThreadTerminate(consumerTid);
        (void)osMessageDelete(ctx.queue);
        return CmsisV1MsgQE2EFail("notStarted", 0);
    }
    if ((osMessagePut(ctx.queue, 0x11U, 0) != osOK) || (osMessagePut(ctx.queue, 0x22U, 0) != osOK)) {
        (void)osThreadTerminate(consumerTid);
        (void)osMessageDelete(ctx.queue);
        return CmsisV1MsgQE2EFail("put", 0);
    }
    for (wait = 0; wait < 50 && ctx.received < 2; wait++) {
        (void)osDelay(1);
    }
    (void)osThreadTerminate(consumerTid);
    (void)osMessageDelete(ctx.queue);
    if ((ctx.received != 2) || (ctx.first != 0x11U) || (ctx.second != 0x22U)) {
        return CmsisV1MsgQE2EFail("content", ctx.received);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] queueE2E\n");
    return 0;
}
