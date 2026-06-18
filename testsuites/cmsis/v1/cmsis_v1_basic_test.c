/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: CMSIS-RTOS v1 basic API test cases for sd3403.
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"

typedef struct {
    volatile uint32_t ran;
    volatile uint32_t arg;
    volatile uint32_t stop;
} CmsisV1ThreadCtx;

typedef struct {
    uint32_t value;
} CmsisV1PoolItem;

typedef struct {
    uint32_t value;
} CmsisV1MailItem;

static volatile uint32_t g_v1TimerCount;

static int CmsisV1Fail(const char *name, const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v1][FAIL] %s %s=0x%x\n", name, detail, value);
    return 1;
}

static void CmsisV1Worker(const void *argument)
{
    CmsisV1ThreadCtx *ctx = (CmsisV1ThreadCtx *)argument;

    if (ctx != NULL) {
        ctx->arg = 0xA5A5U;
        ctx->ran = 1;
        while (ctx->stop == 0) {
            (void)osDelay(1);
        }
    }
}

static void CmsisV1TimerCb(const void *argument)
{
    (void)argument;
    g_v1TimerCount++;
}

osThreadDef(CmsisV1Worker, osPriorityNormal, 1, 0x1000);
osTimerDef(CmsisV1Timer, CmsisV1TimerCb);
osMutexDef(CmsisV1Mutex);
osSemaphoreDef(CmsisV1Sem);
osPoolDef(CmsisV1Pool, 2, CmsisV1PoolItem);
osMessageQDef(CmsisV1MsgQ, 2, uint32_t);
osMailQDef(CmsisV1MailQ, 2, CmsisV1MailItem);

int CmsisV1SemaphoreE2ETest(void);
int CmsisV1MutexE2ETest(void);
int CmsisV1MessageQueueE2ETest(void);
int CmsisV1SignalE2ETest(void);
int CmsisV1TimerE2ETest(void);
int CmsisV1PriorityE2ETest(void);
int CmsisV1SuspendResumeE2ETest(void);

static int CmsisV1KernelTest(void)
{
    if (osKernelRunning() != 1) {
        return CmsisV1Fail("kernel", "running", (uint32_t)osKernelRunning());
    }
    if (osKernelSysTick() == 0) {
        return CmsisV1Fail("kernel", "tick", 0);
    }
    if (osKernelStart() != osOK) {
        return CmsisV1Fail("kernel", "start", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] kernel\n");
    return 0;
}

static int CmsisV1ThreadTest(void)
{
    CmsisV1ThreadCtx ctx = {0};
    osThreadId tid;
    uint32_t wait;

    tid = osThreadCreate(osThread(CmsisV1Worker), &ctx);
    if (tid == NULL) {
        return CmsisV1Fail("thread", "create", 0);
    }
    for (wait = 0; wait < 20 && ctx.ran == 0; wait++) {
        (void)osDelay(1);
    }
    if ((ctx.ran != 1) || (ctx.arg != 0xA5A5U)) {
        (void)osThreadTerminate(tid);
        return CmsisV1Fail("thread", "run", ctx.arg);
    }
    if (osThreadSetPriority(tid, osPriorityAboveNormal) != osOK) {
        (void)osThreadTerminate(tid);
        return CmsisV1Fail("thread", "setPrio", 0);
    }
    if (osThreadGetPriority(tid) != osPriorityAboveNormal) {
        (void)osThreadTerminate(tid);
        return CmsisV1Fail("thread", "getPrio", (uint32_t)osThreadGetPriority(tid));
    }
    if (osThreadTerminate(tid) != osOK) {
        ctx.stop = 1;
        return CmsisV1Fail("thread", "terminate", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] thread\n");
    return 0;
}

static int CmsisV1SignalTest(void)
{
    osThreadId self = osThreadGetId();
    osEvent event;

    if (self == NULL) {
        return CmsisV1Fail("signal", "self", 0);
    }
    if (osSignalSet(self, 0x3) < 0) {
        return CmsisV1Fail("signal", "set", 0);
    }
    event = osSignalWait(0x1, 0);
    if ((event.status != osEventSignal) || (((uint32_t)event.value.signals & 0x1U) == 0)) {
        return CmsisV1Fail("signal", "wait1", (uint32_t)event.status);
    }
    event = osSignalWait(0x2, 0);
    if ((event.status != osEventSignal) || (((uint32_t)event.value.signals & 0x2U) == 0)) {
        return CmsisV1Fail("signal", "wait2", (uint32_t)event.status);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] signal\n");
    return 0;
}

static int CmsisV1SemaphoreTest(void)
{
    osSemaphoreId sem = osSemaphoreCreate(osSemaphore(CmsisV1Sem), 1);

    if (sem == NULL) {
        return CmsisV1Fail("semaphore", "create", 0);
    }
    if (osSemaphoreWait(sem, 0) < 0) {
        (void)osSemaphoreDelete(sem);
        return CmsisV1Fail("semaphore", "wait", 0);
    }
    if (osSemaphoreWait(sem, 0) >= 0) {
        (void)osSemaphoreDelete(sem);
        return CmsisV1Fail("semaphore", "empty", 0);
    }
    if (osSemaphoreRelease(sem) != osOK) {
        (void)osSemaphoreDelete(sem);
        return CmsisV1Fail("semaphore", "release", 0);
    }
    if (osSemaphoreDelete(sem) != osOK) {
        return CmsisV1Fail("semaphore", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] semaphore\n");
    return 0;
}

static int CmsisV1MutexTest(void)
{
    osMutexId mutex = osMutexCreate(osMutex(CmsisV1Mutex));

    if (mutex == NULL) {
        return CmsisV1Fail("mutex", "create", 0);
    }
    if (osMutexWait(mutex, 0) != osOK) {
        (void)osMutexDelete(mutex);
        return CmsisV1Fail("mutex", "wait", 0);
    }
    if (osMutexRelease(mutex) != osOK) {
        (void)osMutexDelete(mutex);
        return CmsisV1Fail("mutex", "release", 0);
    }
    if (osMutexDelete(mutex) != osOK) {
        return CmsisV1Fail("mutex", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] mutex\n");
    return 0;
}

static int CmsisV1PoolTest(void)
{
    osPoolId pool = osPoolCreate(osPool(CmsisV1Pool));
    CmsisV1PoolItem *item1;
    CmsisV1PoolItem *item2;

    if (pool == NULL) {
        return CmsisV1Fail("pool", "create", 0);
    }
    item1 = (CmsisV1PoolItem *)osPoolAlloc(pool);
    item2 = (CmsisV1PoolItem *)osPoolCAlloc(pool);
    if ((item1 == NULL) || (item2 == NULL)) {
        (void)osPoolDelete(pool);
        return CmsisV1Fail("pool", "alloc", 0);
    }
    if (osPoolAlloc(pool) != NULL) {
        (void)osPoolFree(pool, item1);
        (void)osPoolFree(pool, item2);
        (void)osPoolDelete(pool);
        return CmsisV1Fail("pool", "overflow", 0);
    }
    if ((osPoolFree(pool, item1) != osOK) || (osPoolFree(pool, item2) != osOK)) {
        (void)osPoolDelete(pool);
        return CmsisV1Fail("pool", "free", 0);
    }
    if (osPoolDelete(pool) != osOK) {
        return CmsisV1Fail("pool", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] pool\n");
    return 0;
}

static int CmsisV1MessageTest(void)
{
    osMessageQId queue = osMessageCreate(osMessageQ(CmsisV1MsgQ), NULL);
    osEvent event;

    if (queue == NULL) {
        return CmsisV1Fail("message", "create", 0);
    }
    if (osMessagePut(queue, 0x12345678U, 0) != osOK) {
        (void)osMessageDelete(queue);
        return CmsisV1Fail("message", "put", 0);
    }
    event = osMessageGet(queue, 0);
    if ((event.status != osEventMessage) || (event.value.v != 0x12345678U)) {
        (void)osMessageDelete(queue);
        return CmsisV1Fail("message", "get", (uint32_t)event.status);
    }
    if (osMessageDelete(queue) != osOK) {
        return CmsisV1Fail("message", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] message\n");
    return 0;
}

static int CmsisV1MailTest(void)
{
    osMailQId queue = osMailCreate(osMailQ(CmsisV1MailQ), NULL);
    CmsisV1MailItem *mail;
    osEvent event;

    if (queue == NULL) {
        return CmsisV1Fail("mail", "create", 0);
    }
    mail = (CmsisV1MailItem *)osMailCAlloc(queue, 0);
    if (mail == NULL) {
        (void)osMailDelete(queue);
        return CmsisV1Fail("mail", "alloc", 0);
    }
    mail->value = 0x3403U;
    if (osMailPut(queue, mail) != osOK) {
        (void)osMailFree(queue, mail);
        (void)osMailDelete(queue);
        return CmsisV1Fail("mail", "put", 0);
    }
    event = osMailGet(queue, 0);
    if ((event.status != osEventMail) || (((CmsisV1MailItem *)event.value.p)->value != 0x3403U)) {
        (void)osMailDelete(queue);
        return CmsisV1Fail("mail", "get", (uint32_t)event.status);
    }
    if (osMailFree(queue, event.value.p) != osOK) {
        (void)osMailDelete(queue);
        return CmsisV1Fail("mail", "free", 0);
    }
    if (osMailDelete(queue) != osOK) {
        return CmsisV1Fail("mail", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] mail\n");
    return 0;
}

static int CmsisV1TimerTest(void)
{
    osTimerId timer;
    uint32_t wait;

    g_v1TimerCount = 0;
    timer = osTimerCreate(osTimer(CmsisV1Timer), osTimerOnce, NULL);
    if (timer == NULL) {
        return CmsisV1Fail("timer", "create", 0);
    }
    if (osTimerStart(timer, 20) != osOK) {
        (void)osTimerDelete(timer);
        return CmsisV1Fail("timer", "start", 0);
    }
    for (wait = 0; wait < 50 && g_v1TimerCount == 0; wait++) {
        (void)osDelay(1);
    }
    if (osTimerDelete(timer) != osOK) {
        return CmsisV1Fail("timer", "delete", 0);
    }
    if (g_v1TimerCount == 0) {
        return CmsisV1Fail("timer", "callback", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] timer\n");
    return 0;
}

static int CmsisV1DelayTest(void)
{
    uint32_t start = osKernelSysTick();
    osStatus status = osDelay(1);

    if ((status != osOK) && (status != osEventTimeout)) {
        return CmsisV1Fail("delay", "status", (uint32_t)status);
    }
    if (osKernelSysTick() < start) {
        return CmsisV1Fail("delay", "tick", start);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] delay\n");
    return 0;
}

static int CmsisV1NegativePathTest(void)
{
    if (osThreadCreate(NULL, NULL) != NULL) {
        return CmsisV1Fail("negativePaths", "threadCreate", 0);
    }
    if (osThreadTerminate(NULL) != osErrorParameter) {
        return CmsisV1Fail("negativePaths", "threadTerminate", 0);
    }
    if (osSignalSet(NULL, 1) >= 0) {
        return CmsisV1Fail("negativePaths", "signalSet", 0);
    }
    if (osSemaphoreCreate(NULL, 1) != NULL) {
        return CmsisV1Fail("negativePaths", "semCreate", 0);
    }
    if (osSemaphoreWait(NULL, 0) >= 0) {
        return CmsisV1Fail("negativePaths", "semWait", 0);
    }
    if (osMutexCreate(NULL) != NULL) {
        return CmsisV1Fail("negativePaths", "mutexCreate", 0);
    }
    if (osMutexWait(NULL, 0) != osErrorParameter) {
        return CmsisV1Fail("negativePaths", "mutexWait", 0);
    }
    if (osPoolCreate(NULL) != NULL) {
        return CmsisV1Fail("negativePaths", "poolCreate", 0);
    }
    if (osMessageCreate(NULL, NULL) != NULL) {
        return CmsisV1Fail("negativePaths", "messageCreate", 0);
    }
    if (osMailCreate(NULL, NULL) != NULL) {
        return CmsisV1Fail("negativePaths", "mailCreate", 0);
    }
    if (osTimerCreate(NULL, osTimerOnce, NULL) != NULL) {
        return CmsisV1Fail("negativePaths", "timerCreate", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v1][OK] negativePaths\n");
    return 0;
}

void cmsis_test(void)
{
    uint32_t fails = 0;

    CMSIS_TEST_LOG("[cmsis-v1][TEST] start\n");

    fails += (uint32_t)CmsisV1KernelTest();
    fails += (uint32_t)CmsisV1ThreadTest();
    fails += (uint32_t)CmsisV1SignalTest();
    fails += (uint32_t)CmsisV1SemaphoreTest();
    fails += (uint32_t)CmsisV1MutexTest();
    fails += (uint32_t)CmsisV1PoolTest();
    fails += (uint32_t)CmsisV1MessageTest();
    fails += (uint32_t)CmsisV1MailTest();
    fails += (uint32_t)CmsisV1TimerTest();
    fails += (uint32_t)CmsisV1DelayTest();
    fails += (uint32_t)CmsisV1NegativePathTest();

    fails += (uint32_t)CmsisV1SemaphoreE2ETest();
    fails += (uint32_t)CmsisV1MutexE2ETest();
    fails += (uint32_t)CmsisV1MessageQueueE2ETest();
    fails += (uint32_t)CmsisV1SignalE2ETest();
    fails += (uint32_t)CmsisV1TimerE2ETest();
    fails += (uint32_t)CmsisV1PriorityE2ETest();
    fails += (uint32_t)CmsisV1SuspendResumeE2ETest();

    if (fails == 0) {
        CMSIS_TEST_LOG("[cmsis-v1][PASS] tick=%u\n", osKernelSysTick());
    } else {
        CMSIS_TEST_LOG("[cmsis-v1][FAIL] total=%u\n", fails);
    }
}
