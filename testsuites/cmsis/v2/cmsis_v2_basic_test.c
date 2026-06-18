/*
 * Copyright (c) 2026-2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2026-06-16
 * Description: CMSIS-RTOS v2 basic API test cases for sd3403.
 */

#include "cmsis_os.h"
#include "cmsis_test_log.h"
#include "string.h"

typedef struct {
    volatile uint32_t ran;
    volatile uint32_t arg;
    volatile uint32_t stop;
} CmsisThreadCtx;

static void CmsisWorker(void *argument)
{
    CmsisThreadCtx *ctx = (CmsisThreadCtx *)argument;

    if (ctx != NULL) {
        ctx->arg = 0x5a5aU;
        ctx->ran = 1;
        while (ctx->stop == 0) {
            (void)osDelay(1);
        }
    }
}

static int CmsisFail(const char *name, const char *detail, uint32_t value)
{
    CMSIS_TEST_LOG("[cmsis-v2][FAIL] %s %s=0x%x\n", name, detail, value);
    return 1;
}

int CmsisSemaphoreE2ETest(void);
int CmsisMutexE2ETest(void);
int CmsisMessageQueueE2ETest(void);
int CmsisEventFlagsE2ETest(void);
int CmsisThreadFlagsE2ETest(void);
int CmsisTimerE2ETest(void);
int CmsisPriorityE2ETest(void);
int CmsisSuspendResumeE2ETest(void);
int CmsisKernelLockE2ETest(void);

static int CmsisKernelTest(void)
{
    osVersion_t version;
    char id[16];
    int32_t oldLock;

    if (osKernelGetInfo(&version, id, sizeof(id)) != osOK) {
        return CmsisFail("kernel", "getInfo", 0);
    }
    if (strcmp(id, "UniProton") != 0) {
        return CmsisFail("kernel", "id", (uint32_t)id[0]);
    }
    if ((osKernelGetTickFreq() == 0) || (osKernelGetSysTimerFreq() == 0)) {
        return CmsisFail("kernel", "freq", 0);
    }
    if (osMs2Tick(1000) != osKernelGetTickFreq()) {
        return CmsisFail("kernel", "ms2tick", (uint32_t)osMs2Tick(1000));
    }
    if (osKernelGetSysTimerCount() == 0) {
        return CmsisFail("kernel", "sysTimer", 0);
    }
    oldLock = osKernelLock();
    if ((oldLock < 0) || (osKernelGetState() != osKernelLocked)) {
        (void)osKernelUnlock();
        return CmsisFail("kernel", "lock", (uint32_t)oldLock);
    }
    if (osKernelUnlock() <= 0) {
        return CmsisFail("kernel", "unlock", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] kernel\n");
    return 0;
}

static int CmsisThreadTest(void)
{
    CmsisThreadCtx ctx = {0};
    osThreadAttr_t attr = {0};
    osThreadId_t tid;
    uint32_t wait;

    attr.name = "cmsisWorker";
    attr.priority = osPriorityNormal;
    attr.stack_size = 0x1000;
    tid = osThreadNew(CmsisWorker, &ctx, &attr);
    if (tid == NULL) {
        return CmsisFail("thread", "new", 0);
    }
    for (wait = 0; wait < 20 && ctx.ran == 0; wait++) {
        (void)osDelay(1);
    }
    if ((ctx.ran != 1) || (ctx.arg != 0x5a5aU)) {
        (void)osThreadTerminate(tid);
        return CmsisFail("thread", "run", ctx.arg);
    }
    if (osThreadGetName(tid) == NULL) {
        (void)osThreadTerminate(tid);
        return CmsisFail("thread", "name", 0);
    }
    if (osThreadGetStackSize(tid) == 0) {
        (void)osThreadTerminate(tid);
        return CmsisFail("thread", "stack", 0);
    }
    if (osThreadGetStackSpace(tid) == 0) {
        (void)osThreadTerminate(tid);
        return CmsisFail("thread", "stackSpace", 0);
    }
    if (osThreadSetPriority(tid, osPriorityAboveNormal) != osOK) {
        (void)osThreadTerminate(tid);
        return CmsisFail("thread", "setPrio", 0);
    }
    if (osThreadGetPriority(tid) != osPriorityAboveNormal) {
        (void)osThreadTerminate(tid);
        return CmsisFail("thread", "getPrio", (uint32_t)osThreadGetPriority(tid));
    }
    if (osThreadTerminate(tid) != osOK) {
        ctx.stop = 1;
        return CmsisFail("thread", "terminate", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] thread\n");
    return 0;
}

static int CmsisThreadFlagsTest(void)
{
    osThreadId_t self = osThreadGetId();
    uint32_t flags;

    if (self == NULL) {
        return CmsisFail("threadFlags", "self", 0);
    }
    flags = osThreadFlagsSet(self, 0x3U);
    if ((flags & 0x3U) != 0x3U) {
        return CmsisFail("threadFlags", "set", flags);
    }
    flags = osThreadFlagsWait(0x1U, osFlagsWaitAny, 0);
    if ((flags & 0x1U) != 0x1U) {
        return CmsisFail("threadFlags", "waitAny", flags);
    }
    flags = osThreadFlagsWait(0x2U, osFlagsWaitAll, 0);
    if ((flags & 0x2U) != 0x2U) {
        return CmsisFail("threadFlags", "waitAll", flags);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] threadFlags\n");
    return 0;
}

static int CmsisSemaphoreTest(void)
{
    osSemaphoreId_t sem = osSemaphoreNew(2, 1, NULL);

    if (sem == NULL) {
        return CmsisFail("semaphore", "new", 0);
    }
    if (osSemaphoreGetCount(sem) != 1) {
        (void)osSemaphoreDelete(sem);
        return CmsisFail("semaphore", "count1", osSemaphoreGetCount(sem));
    }
    if (osSemaphoreAcquire(sem, 0) != osOK) {
        (void)osSemaphoreDelete(sem);
        return CmsisFail("semaphore", "acquire", 0);
    }
    if (osSemaphoreAcquire(sem, 0) != osErrorTimeout) {
        (void)osSemaphoreDelete(sem);
        return CmsisFail("semaphore", "timeout", 0);
    }
    if ((osSemaphoreRelease(sem) != osOK) || (osSemaphoreGetCount(sem) != 1)) {
        (void)osSemaphoreDelete(sem);
        return CmsisFail("semaphore", "release", osSemaphoreGetCount(sem));
    }
    if ((osSemaphoreRelease(sem) != osOK) || (osSemaphoreGetCount(sem) != 2)) {
        (void)osSemaphoreDelete(sem);
        return CmsisFail("semaphore", "releaseMax", osSemaphoreGetCount(sem));
    }
    if (osSemaphoreRelease(sem) != osErrorResource) {
        (void)osSemaphoreDelete(sem);
        return CmsisFail("semaphore", "overflow", osSemaphoreGetCount(sem));
    }
    if (osSemaphoreDelete(sem) != osOK) {
        return CmsisFail("semaphore", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] semaphore\n");
    return 0;
}

static int CmsisMutexTest(void)
{
    osMutexId_t mutex = osMutexNew(NULL);

    if (mutex == NULL) {
        return CmsisFail("mutex", "new", 0);
    }
    if (osMutexAcquire(mutex, 0) != osOK) {
        (void)osMutexDelete(mutex);
        return CmsisFail("mutex", "acquire", 0);
    }
    if (osMutexRelease(mutex) != osOK) {
        (void)osMutexDelete(mutex);
        return CmsisFail("mutex", "release", 0);
    }
    if (osMutexDelete(mutex) != osOK) {
        return CmsisFail("mutex", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] mutex\n");
    return 0;
}

static int CmsisMemoryPoolTest(void)
{
    osMemoryPoolId_t pool = osMemoryPoolNew(2, 24, NULL);
    void *block1;
    void *block2;
    void *block3;

    if (pool == NULL) {
        return CmsisFail("mempool", "new", 0);
    }
    if (osMemoryPoolGetCapacity(pool) != 2) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "capacity", osMemoryPoolGetCapacity(pool));
    }
    if (osMemoryPoolGetBlockSize(pool) != 24) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "blockSize", osMemoryPoolGetBlockSize(pool));
    }
    if (osMemoryPoolGetCount(pool) != 0) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "count0", osMemoryPoolGetCount(pool));
    }
    block1 = osMemoryPoolAlloc(pool, 0);
    if (block1 == NULL) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "alloc1", 0);
    }
    block2 = osMemoryPoolAlloc(pool, 0);
    if (block2 == NULL) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "alloc2", 0);
    }
    if (osMemoryPoolGetCount(pool) != 2) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "count2", osMemoryPoolGetCount(pool));
    }
    block3 = osMemoryPoolAlloc(pool, 0);
    if (block3 != NULL) {
        (void)osMemoryPoolFree(pool, block3);
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "overflow", 0);
    }
    if (osMemoryPoolFree(pool, block1) != osOK) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "free1", 0);
    }
    if (osMemoryPoolGetSpace(pool) != 1) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "space", osMemoryPoolGetSpace(pool));
    }
    if (osMemoryPoolFree(pool, block2) != osOK) {
        (void)osMemoryPoolDelete(pool);
        return CmsisFail("mempool", "free2", 0);
    }
    if (osMemoryPoolDelete(pool) != osOK) {
        return CmsisFail("mempool", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] mempool\n");
    return 0;
}

static int CmsisEventFlagsTest(void)
{
    osEventFlagsId_t ef = osEventFlagsNew(NULL);
    uint32_t flags;

    if (ef == NULL) {
        return CmsisFail("eventFlags", "new", 0);
    }
    if (osEventFlagsGet(ef) != 0) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "get0", osEventFlagsGet(ef));
    }
    flags = osEventFlagsSet(ef, 0x5U);
    if ((flags & 0x5U) != 0x5U) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "set1", flags);
    }
    flags = osEventFlagsSet(ef, 0x3U);
    if ((flags & 0x7U) != 0x7U) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "set2", flags);
    }
    if (osEventFlagsGet(ef) != 0x7U) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "get7", osEventFlagsGet(ef));
    }
    flags = osEventFlagsWait(ef, 0x5U, osFlagsWaitAny, 0);
    if ((flags & 0x5U) != 0x5U) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "waitAny", flags);
    }
    if (osEventFlagsGet(ef) != 0x2U) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "afterWait", osEventFlagsGet(ef));
    }
    flags = osEventFlagsClear(ef, 0x2U);
    if ((flags & 0x2U) != 0x2U) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "clear", flags);
    }
    if (osEventFlagsGet(ef) != 0) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "afterClear", osEventFlagsGet(ef));
    }
    flags = osEventFlagsWait(ef, 0x1U, osFlagsWaitAny, 0);
    if (flags != osFlagsErrorResource) {
        (void)osEventFlagsDelete(ef);
        return CmsisFail("eventFlags", "waitEmpty", flags);
    }
    if (osEventFlagsDelete(ef) != osOK) {
        return CmsisFail("eventFlags", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] eventFlags\n");
    return 0;
}

static int CmsisMessageQueueTest(void)
{
    osMessageQueueId_t queue = osMessageQueueNew(2, sizeof(uint32_t), NULL);
    uint32_t value1 = 0x3403;
    uint32_t value2 = 0x2026;
    uint32_t out = 0;

    if (queue == NULL) {
        return CmsisFail("queue", "new", 0);
    }
    if ((osMessageQueueGetCapacity(queue) != 2) || (osMessageQueueGetMsgSize(queue) != sizeof(uint32_t))) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("queue", "attrs", osMessageQueueGetCapacity(queue));
    }
    if ((osMessageQueuePut(queue, &value1, 0, 0) != osOK) || (osMessageQueuePut(queue, &value2, 0, 0) != osOK)) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("queue", "put", osMessageQueueGetCount(queue));
    }
    if ((osMessageQueueGetCount(queue) != 2) || (osMessageQueueGetSpace(queue) != 0)) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("queue", "count", osMessageQueueGetCount(queue));
    }
    if ((osMessageQueueGet(queue, &out, NULL, 0) != osOK) || (out != value1)) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("queue", "get", out);
    }
    if (osMessageQueueReset(queue) != osOK || osMessageQueueGetCount(queue) != 0) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("queue", "reset", osMessageQueueGetCount(queue));
    }
    if (osMessageQueueGet(queue, &out, NULL, 0) != osErrorTimeout) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("queue", "empty", out);
    }
    if (osMessageQueueDelete(queue) != osOK) {
        return CmsisFail("queue", "delete", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] queue\n");
    return 0;
}

static int CmsisDelayTest(void)
{
    uint64_t start = osKernelGetTickCount();

    if (osDelay(1) != osOK) {
        return CmsisFail("delay", "delay", 0);
    }
    if (osKernelGetTickCount() < start) {
        return CmsisFail("delay", "tick", (uint32_t)start);
    }
    if (osDelayUntil(osKernelGetTickCount()) != osError) {
        return CmsisFail("delay", "untilPast", 0);
    }
    CMSIS_TEST_LOG("[cmsis-v2][OK] delay\n");
    return 0;
}

static int CmsisNegativePathTest(void)
{
    osVersion_t version;
    char id[16];
    osThreadId_t self = osThreadGetId();
    osMessageQueueId_t queue;
    uint32_t value = 0x55aaU;

    if (osKernelGetInfo(NULL, id, sizeof(id)) != osErrorParameter) {
        return CmsisFail("negativePaths", "kernelInfoVersion", 0);
    }
    if (osKernelGetInfo(&version, NULL, sizeof(id)) != osErrorParameter) {
        return CmsisFail("negativePaths", "kernelInfoId", 0);
    }
    if (osThreadNew(NULL, NULL, NULL) != NULL) {
        return CmsisFail("negativePaths", "threadNew", 0);
    }
    if (osThreadSetPriority(NULL, osPriorityNormal) != osErrorParameter) {
        return CmsisFail("negativePaths", "threadPrioNull", 0);
    }
    if ((self != NULL) && (osThreadSetPriority(self, osPriorityReserved) != osErrorParameter)) {
        return CmsisFail("negativePaths", "threadPrioValue", 0);
    }
    if (osThreadFlagsSet(NULL, 1U) != osFlagsErrorParameter) {
        return CmsisFail("negativePaths", "flagsSetNull", 0);
    }
    if ((self != NULL) && (osThreadFlagsSet(self, 0U) != osFlagsErrorParameter)) {
        return CmsisFail("negativePaths", "flagsSetZero", 0);
    }
    if (osThreadFlagsWait(0U, osFlagsWaitAny, 0) != osFlagsErrorParameter) {
        return CmsisFail("negativePaths", "flagsWaitZero", 0);
    }
    if (osSemaphoreNew(0, 0, NULL) != NULL) {
        return CmsisFail("negativePaths", "semNewZero", 0);
    }
    if (osSemaphoreNew(1, 2, NULL) != NULL) {
        return CmsisFail("negativePaths", "semNewCount", 0);
    }
    if (osSemaphoreAcquire(NULL, 0) != osErrorParameter) {
        return CmsisFail("negativePaths", "semAcquireNull", 0);
    }
    if (osSemaphoreDelete(NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "semDeleteNull", 0);
    }
    if (osMutexAcquire(NULL, 0) != osErrorParameter) {
        return CmsisFail("negativePaths", "mutexAcquireNull", 0);
    }
    if (osMutexRelease(NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "mutexReleaseNull", 0);
    }
    if (osMutexDelete(NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "mutexDeleteNull", 0);
    }
    if (osMemoryPoolNew(0, 24, NULL) != NULL) {
        return CmsisFail("negativePaths", "poolNewCount", 0);
    }
    if (osMemoryPoolNew(2, 0, NULL) != NULL) {
        return CmsisFail("negativePaths", "poolNewSize", 0);
    }
    if (osMemoryPoolAlloc(NULL, 0) != NULL) {
        return CmsisFail("negativePaths", "poolAllocNull", 0);
    }
    if (osMemoryPoolFree(NULL, NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "poolFreeNull", 0);
    }
    if (osMemoryPoolDelete(NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "poolDeleteNull", 0);
    }
    if (osMessageQueueNew(0, sizeof(uint32_t), NULL) != NULL) {
        return CmsisFail("negativePaths", "queueNewCount", 0);
    }
    if (osMessageQueueNew(1, 0, NULL) != NULL) {
        return CmsisFail("negativePaths", "queueNewSize", 0);
    }
    if (osMessageQueuePut(NULL, &value, 0, 0) != osErrorParameter) {
        return CmsisFail("negativePaths", "queuePutNull", 0);
    }
    if (osMessageQueueGet(NULL, &value, NULL, 0) != osErrorParameter) {
        return CmsisFail("negativePaths", "queueGetNull", 0);
    }
    if (osMessageQueueReset(NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "queueResetNull", 0);
    }
    if (osMessageQueueDelete(NULL) != osErrorParameter) {
        return CmsisFail("negativePaths", "queueDeleteNull", 0);
    }
    queue = osMessageQueueNew(1, sizeof(uint32_t), NULL);
    if (queue == NULL) {
        return CmsisFail("negativePaths", "queueNew", 0);
    }
    if (osMessageQueuePut(queue, NULL, 0, 0) != osErrorParameter) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("negativePaths", "queuePutMsg", 0);
    }
    if (osMessageQueueGet(queue, NULL, NULL, 0) != osErrorParameter) {
        (void)osMessageQueueDelete(queue);
        return CmsisFail("negativePaths", "queueGetMsg", 0);
    }
    if (osMessageQueueDelete(queue) != osOK) {
        return CmsisFail("negativePaths", "queueDelete", 0);
    }
    if (osTimerNew(NULL, osTimerOnce, NULL, NULL) != NULL) {
        return CmsisFail("negativePaths", "timerNewFunc", 0);
    }
    if (osTimerStart(NULL, 1) != osErrorParameter) {
        return CmsisFail("negativePaths", "timerStartNull", 0);
    }

    CMSIS_TEST_LOG("[cmsis-v2][OK] negativePaths\n");
    return 0;
}

void cmsis_test(void)
{
    uint32_t fails = 0;

    CMSIS_TEST_LOG("[cmsis-v2][TEST] start\n");

    /* Basic API tests */
    fails += (uint32_t)CmsisKernelTest();
    fails += (uint32_t)CmsisThreadTest();
    fails += (uint32_t)CmsisThreadFlagsTest();
    fails += (uint32_t)CmsisSemaphoreTest();
    fails += (uint32_t)CmsisMutexTest();
    fails += (uint32_t)CmsisMemoryPoolTest();
    fails += (uint32_t)CmsisEventFlagsTest();
    fails += (uint32_t)CmsisMessageQueueTest();
    fails += (uint32_t)CmsisDelayTest();
    fails += (uint32_t)CmsisNegativePathTest();

    /* End-to-end scenario tests */
    fails += (uint32_t)CmsisSemaphoreE2ETest();
    fails += (uint32_t)CmsisMutexE2ETest();
    fails += (uint32_t)CmsisMessageQueueE2ETest();
    fails += (uint32_t)CmsisEventFlagsE2ETest();
    fails += (uint32_t)CmsisThreadFlagsE2ETest();
    fails += (uint32_t)CmsisTimerE2ETest();
    fails += (uint32_t)CmsisPriorityE2ETest();
    fails += (uint32_t)CmsisSuspendResumeE2ETest();
    fails += (uint32_t)CmsisKernelLockE2ETest();

    if (fails == 0) {
        CMSIS_TEST_LOG("[cmsis-v2][PASS] tick=%u\n", (uint32_t)osKernelGetTickCount());
    } else {
        CMSIS_TEST_LOG("[cmsis-v2][FAIL] total=%u\n", fails);
    }
}
