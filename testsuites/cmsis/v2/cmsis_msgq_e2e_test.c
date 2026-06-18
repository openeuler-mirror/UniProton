/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS message queue end-to-end test.
 *
 * Test method:
 * 1. Create a CMSIS message queue with capacity 2.
 * 2. A high-priority producer thread writes two messages, filling the queue,
 *    then attempts to write a third message with osWaitForever.
 * 3. A low-priority consumer waits until the queue is full, delays briefly,
 *    and verifies the producer is still blocked before any read occurs.
 * 4. The consumer reads three messages. Reading one message must unblock the
 *    producer, allowing the third write to complete.
 * 5. The test passes only if producer blocking is observed, all three messages
 *    are transferred, and FIFO order is preserved.
 *
 * Sequence diagram:
 *   Queue capacity = 2
 *
 *   Producer                         Consumer
 *   --------                         --------
 *   put 0x1000 -> queue[0]
 *   put 0x1001 -> queue[1], full
 *   put 0x1002 with osWaitForever
 *       -> blocked
 *                                    verify produced == 2
 *                                    get 0x1000 -> frees one slot
 *   third put completes
 *                                    get 0x1001
 *                                    get 0x1002
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    /* CMSIS message queue shared by producer and consumer threads. */
    osMessageQueueId_t queue;
    /* Number of messages successfully put by the producer. */
    volatile uint32_t produced;
    /* Number of messages successfully read by the consumer. */
    volatile uint32_t consumed;
    /* Set after the producer fills the queue with the first two messages. */
    volatile uint32_t filled;
    /* Set when the consumer observes producer is blocked before any read. */
    volatile uint32_t producerBlockedBeforeRead;
    /* Values read by the consumer; used to verify FIFO order. */
    volatile uint32_t values[4];
} MsgQueueE2ECtx;

static int CmsisMsgQE2EFail(const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] queueE2E %s=0x%x\n", detail, value);
    return 1;
}

static void ProducerTask(void *arg)
{
    MsgQueueE2ECtx *ctx = (MsgQueueE2ECtx *)arg;
    uint32_t msg;

    for (uint32_t i = 0; i < 2; i++) {
        msg = 0x1000 + i;
        if (osMessageQueuePut(ctx->queue, &msg, 0, osWaitForever) == osOK) {
            ctx->produced++;
        }
    }
    ctx->filled = 1;
    msg = 0x1002;
    if (osMessageQueuePut(ctx->queue, &msg, 0, osWaitForever) == osOK) {
        ctx->produced++;
    }
}

static void ConsumerTask(void *arg)
{
    MsgQueueE2ECtx *ctx = (MsgQueueE2ECtx *)arg;
    uint32_t msg;

    while (ctx->filled == 0) {
        (void)osDelay(1);
    }
    (void)osDelay(3);
    ctx->producerBlockedBeforeRead = (ctx->produced == 2) ? 1 : 0;
    for (uint32_t i = 0; i < 3; i++) {
        if (osMessageQueueGet(ctx->queue, &msg, NULL, osWaitForever) == osOK) {
            ctx->values[i] = msg;
            ctx->consumed++;
        }
    }
}

int CmsisMessageQueueE2ETest(void)
{
    MsgQueueE2ECtx ctx = {0};
    osThreadAttr_t prodAttr = {0};
    osThreadAttr_t consAttr = {0};
    osThreadId_t prodTid;
    osThreadId_t consTid;
    uint32_t wait;

    ctx.queue = osMessageQueueNew(2, sizeof(uint32_t), NULL);
    if (ctx.queue == NULL) {
        return CmsisMsgQE2EFail("new", 0);
    }

    prodAttr.name = "producer";
    prodAttr.priority = osPriorityHigh;
    prodAttr.stack_size = 0x1000;
    prodTid = osThreadNew(ProducerTask, &ctx, &prodAttr);

    consAttr.name = "consumer";
    consAttr.priority = osPriorityLow;
    consAttr.stack_size = 0x1000;
    consTid = osThreadNew(ConsumerTask, &ctx, &consAttr);

    for (wait = 0; wait < 50 && (ctx.produced < 3 || ctx.consumed < 3); wait++) {
        (void)osDelay(1);
    }

    (void)osThreadTerminate(prodTid);
    (void)osThreadTerminate(consTid);
    (void)osMessageQueueDelete(ctx.queue);

    if (ctx.producerBlockedBeforeRead != 1) {
        return CmsisMsgQE2EFail("notBlocked", ctx.produced);
    }
    if (ctx.produced != 3) {
        return CmsisMsgQE2EFail("produced", ctx.produced);
    }
    if (ctx.consumed != 3) {
        return CmsisMsgQE2EFail("consumed", ctx.consumed);
    }
    for (uint32_t i = 0; i < 3; i++) {
        if (ctx.values[i] != 0x1000 + i) {
            return CmsisMsgQE2EFail("order", ctx.values[i]);
        }
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] queueE2E\n");
    return 0;
}
