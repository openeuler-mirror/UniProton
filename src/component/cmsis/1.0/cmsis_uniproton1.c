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
 * Description: CMSIS-RTOS v1 adapter for UniProton PRT APIs.
 */

#include "cmsis_uniproton.h"

#if (CMSIS_OS_VER == 1)

struct os_pool_cb {
    U32 itemSize;
    U32 itemCount;
    U32 used;
};

struct os_messageQ_cb {
    U32 queueId;
};

struct os_mailQ_cb {
    U32 queueId;
    U32 itemSize;
    U32 queueSize;
};

typedef struct {
    TimerHandle handle;
    os_ptimer func;
    void *argument;
    os_timer_type type;
    U32 interval;
} CmsisTimerV1;

#define CMSIS_V1_VALID_SIGNAL_MASK ((1U << osFeature_Signals) - 1U)

static osStatus CmsisQueueWriteStatus(U32 ret)
{
    if (ret == OS_OK) {
        return osOK;
    }
    if ((ret == OS_ERRNO_QUEUE_TIMEOUT) || (ret == OS_ERRNO_QUEUE_NO_SOURCE)) {
        return osErrorTimeoutResource;
    }
    if ((ret == OS_ERRNO_QUEUE_INVALID) || (ret == OS_ERRNO_QUEUE_NOT_CREATE) ||
        (ret == OS_ERRNO_QUEUE_PTR_NULL) || (ret == OS_ERRNO_QUEUE_SIZE_ZERO) ||
        (ret == OS_ERRNO_QUEUE_SIZE_TOO_BIG)) {
        return osErrorParameter;
    }
    return osErrorResource;
}

static osStatus CmsisQueueReadStatus(U32 ret)
{
    if (ret == OS_OK) {
        return osEventMessage;
    }
    if ((ret == OS_ERRNO_QUEUE_TIMEOUT) || (ret == OS_ERRNO_QUEUE_NO_SOURCE)) {
        return osEventTimeout;
    }
    if ((ret == OS_ERRNO_QUEUE_INVALID) || (ret == OS_ERRNO_QUEUE_NOT_CREATE) ||
        (ret == OS_ERRNO_QUEUE_PTR_NULL)) {
        return osErrorParameter;
    }
    return osErrorResource;
}

static osThreadId CmsisHandleToThreadId(TskHandle taskId)
{
    return (osThreadId)(uintptr_t)(taskId + 1U);
}

static TskHandle CmsisThreadIdToHandle(osThreadId threadId)
{
    return (TskHandle)((uintptr_t)threadId - 1U);
}

static osStatus g_v1KernelState = osErrorOS;

osStatus osKernelInitialize(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (g_v1KernelState == osOK) {
        return osErrorOS;
    }
    if (CmsisKernelStarted()) {
        g_v1KernelState = osOK;
        return osOK;
    }
    g_v1KernelState = osOK;
    return osOK;
}

osStatus osKernelStart(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (CmsisKernelStarted()) {
        return osOK;
    }
    return osErrorOS;
}

int32_t osKernelRunning(void)
{
    return CmsisKernelStarted() ? 1 : 0;
}

uint32_t osKernelSysTick(void)
{
    return PRT_TickGetCount();
}

static void CmsisThreadEntry(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    const osThreadDef_t *threadDef = (const osThreadDef_t *)arg0;
    (void)arg2;
    (void)arg3;
    threadDef->pthread((void *)arg1);
}

osThreadId osThreadCreate(const osThreadDef_t *thread_def, void *argument)
{
    TskHandle taskId;

    if ((thread_def == NULL) || (thread_def->pthread == NULL) ||
        !CmsisPrioIsValid((int32_t)thread_def->tpriority) || OS_INT_ACTIVE) {
        return NULL;
    }

    if (CmsisCreateTask(&taskId, CmsisThreadEntry, (uintptr_t)thread_def, (uintptr_t)argument, thread_def->name,
        thread_def->stacksize, CmsisPrioToPrt((int32_t)thread_def->tpriority)) != OS_OK) {
        return NULL;
    }
    return CmsisHandleToThreadId(taskId);
}

osThreadId osUsrThreadCreate(const osThreadDef_t *thread_def, void *stackPointer, U32 stackSize, void *argument)
{
    (void)stackPointer;
    (void)stackSize;
    return osThreadCreate(thread_def, argument);
}

osThreadId osThreadGetId(void)
{
    TskHandle taskId;
    return (PRT_TaskSelf(&taskId) == OS_OK) ? CmsisHandleToThreadId(taskId) : NULL;
}

osStatus osThreadTerminate(osThreadId thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskDelete(CmsisThreadIdToHandle(thread_id)) == OS_OK) ? osOK : osErrorResource;
}

osStatus osThreadSelfSuspend(void)
{
    TskHandle taskId;
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return ((PRT_TaskSelf(&taskId) == OS_OK) && (PRT_TaskSuspend(taskId) == OS_OK)) ? osOK : osErrorOS;
}

osStatus osThreadResume(osThreadId thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskResume(CmsisThreadIdToHandle(thread_id)) == OS_OK) ? osOK : osErrorResource;
}

osStatus osThreadYield(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskDelay(0) == OS_OK) ? osOK : osErrorOS;
}

osStatus osThreadSetPriority(osThreadId thread_id, osPriority priority)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (!CmsisPrioIsValid((int32_t)priority)) {
        return osErrorValue;
    }
    return (PRT_TaskSetPriority(CmsisThreadIdToHandle(thread_id), CmsisPrioToPrt((int32_t)priority)) == OS_OK) ?
        osOK : osErrorResource;
}

osPriority osThreadGetPriority(osThreadId thread_id)
{
    TskPrior prio;
    int32_t cmsisPrio;
    if ((thread_id == NULL) || OS_INT_ACTIVE || (PRT_TaskGetPriority(CmsisThreadIdToHandle(thread_id), &prio) != OS_OK)) {
        return osPriorityError;
    }
    cmsisPrio = CmsisPrioFromPrt(prio);
    if (cmsisPrio < 0) {
        return osPriorityError;
    }
    return (osPriority)cmsisPrio;
}

osSemaphoreId osSemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count)
{
    SemHandle sem;
    if ((semaphore_def == NULL) || (count < 0) || ((U32)count > OS_SEM_COUNT_MAX) || OS_INT_ACTIVE ||
        (PRT_SemCreate((U32)count, &sem) != OS_OK)) {
        return NULL;
    }
    return (osSemaphoreId)(uintptr_t)sem;
}

static void CmsisTimerCallback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    CmsisTimerV1 *timer = (CmsisTimerV1 *)(uintptr_t)arg1;
    (void)tmrHandle;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    if ((timer != NULL) && (timer->func != NULL)) {
        timer->func(timer->argument);
    }
}

osTimerId osTimerCreate(const osTimerDef_t *timer_def, os_timer_type type, void *argument)
{
    struct TimerCreatePara para = {0};
    CmsisTimerV1 *timer;

    if ((timer_def == NULL) || (timer_def->ptimer == NULL) || OS_INT_ACTIVE ||
        ((type != osTimerOnce) && (type != osTimerPeriodic) && (type != osTimerDelay))) {
        return NULL;
    }
    timer = (CmsisTimerV1 *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisTimerV1));
    if (timer == NULL) {
        return NULL;
    }
    timer->func = timer_def->ptimer;
    timer->argument = argument;
    timer->type = type;
    timer->interval = CmsisTickToMs(1);

    para.type = OS_TIMER_SOFTWARE;
    para.mode = (type == osTimerPeriodic) ? OS_TIMER_LOOP : OS_TIMER_ONCE;
    para.interval = timer->interval;
    para.callBackFunc = CmsisTimerCallback;
    para.arg1 = (U32)(uintptr_t)timer;
    if (PRT_TimerCreate(&para, &timer->handle) != OS_OK) {
        (void)PRT_MemFree(0, timer);
        return NULL;
    }
    return (osTimerId)timer;
}

osStatus osTimerStart(osTimerId timer_id, uint32_t millisec)
{
    CmsisTimerV1 *timer = (CmsisTimerV1 *)timer_id;
    struct TimerCreatePara para = {0};
    U32 interval = millisec;
    if (timer == NULL) {
        return osErrorParameter;
    }
    if ((interval == 0) || OS_INT_ACTIVE) {
        return OS_INT_ACTIVE ? osErrorISR : osErrorParameter;
    }
    if (timer->interval != interval) {
        (void)PRT_TimerDelete(0, timer->handle);
        para.type = OS_TIMER_SOFTWARE;
        para.mode = (timer->type == osTimerPeriodic) ? OS_TIMER_LOOP : OS_TIMER_ONCE;
        para.interval = interval;
        para.callBackFunc = CmsisTimerCallback;
        para.arg1 = (U32)(uintptr_t)timer;
        if (PRT_TimerCreate(&para, &timer->handle) != OS_OK) {
            return osErrorResource;
        }
        timer->interval = interval;
    }
    return (PRT_TimerStart(0, timer->handle) == OS_OK) ? osOK : osErrorResource;
}

osStatus osTimerStop(osTimerId timer_id)
{
    CmsisTimerV1 *timer = (CmsisTimerV1 *)timer_id;
    if (timer == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TimerStop(0, timer->handle) == OS_OK) ? osOK : osErrorResource;
}

osStatus osTimerRestart(osTimerId timer_id, uint32_t millisec, uint8_t strict)
{
    osStatus status;

    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    status = osTimerStop(timer_id);
    if (strict && (status != osOK)) {
        return status;
    }
    return osTimerStart(timer_id, millisec);
}

osStatus osTimerDelete(osTimerId timer_id)
{
    CmsisTimerV1 *timer = (CmsisTimerV1 *)timer_id;
    if (timer == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (PRT_TimerDelete(0, timer->handle) != OS_OK) {
        return osErrorResource;
    }
    (void)PRT_MemFree(0, timer);
    return osOK;
}

osMutexId osMutexCreate(const osMutexDef_t *mutex_def)
{
    SemHandle sem;
    if ((mutex_def == NULL) || OS_INT_ACTIVE) {
        return NULL;
    }
#if defined(OS_OPTION_BIN_SEM)
    return (PRT_SemMutexCreate(&sem) == OS_OK) ? (osMutexId)(uintptr_t)sem : NULL;
#else
    return NULL;
#endif
}

osStatus osMutexWait(osMutexId mutex_id, uint32_t millisec)
{
    if (mutex_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_SemPend((SemHandle)(uintptr_t)mutex_id, CmsisMsToTick(millisec)) == OS_OK) ? osOK : osErrorTimeoutResource;
}

osStatus osMutexRelease(osMutexId mutex_id)
{
    if (mutex_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_SemPost((SemHandle)(uintptr_t)mutex_id) == OS_OK) ? osOK : osErrorResource;
}

osStatus osMutexDelete(osMutexId mutex_id)
{
    if (mutex_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_SemDelete((SemHandle)(uintptr_t)mutex_id) == OS_OK) ? osOK : osErrorResource;
}

osPoolId osPoolCreate(const osPoolDef_t *pool_def)
{
    osPoolId pool;

    if ((pool_def == NULL) || (pool_def->pool_sz == 0) || (pool_def->item_sz == 0) || OS_INT_ACTIVE) {
        return NULL;
    }
    pool = (osPoolId)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(struct os_pool_cb));
    if (pool == NULL) {
        return NULL;
    }
    pool->itemSize = pool_def->item_sz;
    pool->itemCount = pool_def->pool_sz;
    pool->used = 0;
    return pool;
}

void *osPoolAlloc(osPoolId pool_id)
{
    void *block;
    if ((pool_id == NULL) || (pool_id->used >= pool_id->itemCount)) {
        return NULL;
    }
    block = PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, pool_id->itemSize);
    if (block != NULL) {
        pool_id->used++;
    }
    return block;
}

void *osPoolCAlloc(osPoolId pool_id)
{
    void *block = osPoolAlloc(pool_id);
    if (block != NULL) {
        (void)memset_s(block, pool_id->itemSize, 0, pool_id->itemSize);
    }
    return block;
}

osStatus osPoolFree(osPoolId pool_id, void *block)
{
    if ((pool_id == NULL) || (block == NULL) || (PRT_MemFree(0, block) != OS_OK)) {
        return osErrorParameter;
    }
    if (pool_id->used > 0) {
        pool_id->used--;
    }
    return osOK;
}

osStatus osPoolDelete(osPoolId pool_id)
{
    if (pool_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (pool_id->used != 0) {
        return osErrorResource;
    }
    return (PRT_MemFree(0, pool_id) == OS_OK) ? osOK : osErrorValue;
}

osSemaphoreId osBinarySemaphoreCreate(const osSemaphoreDef_t *semaphore_def, int32_t count)
{
    if ((semaphore_def == NULL) || (count < 0) || (count > 1) || OS_INT_ACTIVE) {
        return NULL;
    }
    return osSemaphoreCreate(semaphore_def, count > 0 ? 1 : 0);
}

int32_t osSemaphoreWait(osSemaphoreId semaphore_id, uint32_t millisec)
{
    SemHandle sem = (SemHandle)(uintptr_t)semaphore_id;
    U32 count = 0;
    if ((semaphore_id == NULL) || OS_INT_ACTIVE) {
        return -1;
    }
    if (PRT_SemPend(sem, CmsisMsToTick(millisec)) != OS_OK) {
        return -1;
    }
    (void)PRT_SemGetCount(sem, &count);
    return (int32_t)count;
}

osStatus osSemaphoreRelease(osSemaphoreId semaphore_id)
{
    if (semaphore_id == NULL) {
        return osErrorParameter;
    }
    return (PRT_SemPost((SemHandle)(uintptr_t)semaphore_id) == OS_OK) ? osOK : osErrorResource;
}

osStatus osSemaphoreDelete(osSemaphoreId semaphore_id)
{
    if (semaphore_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_SemDelete((SemHandle)(uintptr_t)semaphore_id) == OS_OK) ? osOK : osErrorResource;
}

osMessageQId osMessageCreate(const osMessageQDef_t *queue_def, osThreadId thread_id)
{
    osMessageQId queue;
    (void)thread_id;
    if ((queue_def == NULL) || (queue_def->queue_sz == 0) || OS_INT_ACTIVE) {
        return NULL;
    }
    queue = (osMessageQId)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(struct os_messageQ_cb));
    if (queue == NULL) {
        return NULL;
    }
    if (PRT_QueueCreate((U16)queue_def->queue_sz, sizeof(U32), &queue->queueId) != OS_OK) {
        (void)PRT_MemFree(0, queue);
        return NULL;
    }
    return queue;
}

static osStatus CmsisMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec, U32 prio)
{
    if ((queue_id == NULL) || (OS_INT_ACTIVE && (millisec != 0))) {
        return osErrorParameter;
    }
    return CmsisQueueWriteStatus(PRT_QueueWrite(queue_id->queueId, &info, sizeof(info), CmsisMsToTick(millisec), prio));
}

osStatus osMessagePutHead(const osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
    return CmsisMessagePut(queue_id, info, millisec, OS_QUEUE_URGENT);
}

osStatus osMessagePut(osMessageQId queue_id, uint32_t info, uint32_t millisec)
{
    return CmsisMessagePut(queue_id, info, millisec, OS_QUEUE_NORMAL);
}

osEvent osMessageGet(osMessageQId queue_id, uint32_t millisec)
{
    osEvent event = {0};
    U32 len = sizeof(U32);
    if ((queue_id == NULL) || (OS_INT_ACTIVE && (millisec != 0))) {
        event.status = osErrorParameter;
        return event;
    }
    event.status = CmsisQueueReadStatus(PRT_QueueRead(queue_id->queueId, &event.value.v, &len, CmsisMsToTick(millisec)));
    return event;
}

osStatus osMessageDelete(const osMessageQId queue_id)
{
    if (queue_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (PRT_QueueDelete(queue_id->queueId) != OS_OK) {
        return osErrorResource;
    }
    (void)PRT_MemFree(0, queue_id);
    return osOK;
}

osMailQId osMailCreate(const osMailQDef_t *queue_def, osThreadId thread_id)
{
    osMailQId mailQ;
    (void)thread_id;

    if ((queue_def == NULL) || (queue_def->queue_sz == 0) || (queue_def->item_sz == 0) || OS_INT_ACTIVE) {
        return NULL;
    }
    mailQ = (osMailQId)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(struct os_mailQ_cb));
    if (mailQ == NULL) {
        return NULL;
    }
    if (PRT_QueueCreate((U16)queue_def->queue_sz, sizeof(void *), &mailQ->queueId) != OS_OK) {
        (void)PRT_MemFree(0, mailQ);
        return NULL;
    }
    mailQ->itemSize = queue_def->item_sz;
    mailQ->queueSize = queue_def->queue_sz;
    return mailQ;
}

void *osMailAlloc(osMailQId queue_id, uint32_t millisec)
{
    (void)millisec;
    return (queue_id == NULL) ? NULL : PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, queue_id->itemSize);
}

void *osMailCAlloc(osMailQId queue_id, uint32_t millisec)
{
    void *mail = osMailAlloc(queue_id, millisec);
    if (mail != NULL) {
        (void)memset_s(mail, queue_id->itemSize, 0, queue_id->itemSize);
    }
    return mail;
}

osStatus osMailPut(osMailQId queue_id, void *mail)
{
    if (queue_id == NULL) {
        return osErrorParameter;
    }
    if (mail == NULL) {
        return osErrorValue;
    }
    return CmsisQueueWriteStatus(PRT_QueueWrite(queue_id->queueId, &mail, sizeof(mail), 0, OS_QUEUE_NORMAL));
}

osStatus osMailPutHead(osMailQId queue_id, void *mail)
{
    if (queue_id == NULL) {
        return osErrorParameter;
    }
    if (mail == NULL) {
        return osErrorValue;
    }
    return CmsisQueueWriteStatus(PRT_QueueWrite(queue_id->queueId, &mail, sizeof(mail), 0, OS_QUEUE_URGENT));
}

osEvent osMailGet(osMailQId queue_id, uint32_t millisec)
{
    osEvent event = {0};
    U32 len = sizeof(void *);

    if ((queue_id == NULL) || (OS_INT_ACTIVE && (millisec != 0))) {
        event.status = osErrorParameter;
        return event;
    }
    event.status = CmsisQueueReadStatus(PRT_QueueRead(queue_id->queueId, &event.value.p, &len, CmsisMsToTick(millisec)));
    if (event.status == osEventMessage) {
        event.status = osEventMail;
        event.def.mail_id = queue_id;
    }
    return event;
}

osStatus osMailFree(osMailQId queue_id, void *mail)
{
    (void)queue_id;
    if (queue_id == NULL) {
        return osErrorParameter;
    }
    if (mail == NULL) {
        return osErrorValue;
    }
    return (PRT_MemFree(0, mail) == OS_OK) ? osOK : osErrorValue;
}

osStatus osMailClear(osMailQId queue_id)
{
    osEvent event;

    if (queue_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    for (;;) {
        event = osMailGet(queue_id, 0);
        if (event.status == osEventMail) {
            (void)osMailFree(queue_id, event.value.p);
        } else if (event.status == osEventTimeout) {
            return osOK;
        } else {
            return event.status;
        }
    }
}

osStatus osMailDelete(osMailQId queue_id)
{
    if (queue_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    (void)osMailClear(queue_id);
    if (PRT_QueueDelete(queue_id->queueId) != OS_OK) {
        return osErrorResource;
    }
    (void)PRT_MemFree(0, queue_id);
    return osOK;
}

int32_t osSignalSet(osThreadId thread_id, int32_t signals)
{
    TskHandle taskId;
    struct TagTskCb *taskCb;
    U32 oldFlags;

    if ((thread_id == NULL) || (signals < 0) || (((U32)signals & ~CMSIS_V1_VALID_SIGNAL_MASK) != 0)) {
        return (int32_t)0x80000000;
    }
    taskId = CmsisThreadIdToHandle(thread_id);
    taskCb = GET_TCB_HANDLE(taskId);
    if ((taskCb == NULL) || TSK_IS_UNUSED(taskCb)) {
        return (int32_t)0x80000000;
    }
#if defined(OS_OPTION_EVENT)
    oldFlags = taskCb->event;
#else
    oldFlags = 0;
#endif
    if (PRT_EventWrite(taskId, (U32)signals) != OS_OK) {
        return (int32_t)0x80000000;
    }
    return (int32_t)oldFlags;
}

int32_t osSignalClear(osThreadId thread_id, int32_t signals)
{
    U32 events = 0;

    if ((thread_id == NULL) || (thread_id != osThreadGetId()) || (signals < 0) ||
        (((U32)signals & ~CMSIS_V1_VALID_SIGNAL_MASK) != 0) || OS_INT_ACTIVE) {
        return (int32_t)0x80000000;
    }
    (void)PRT_EventRead((U32)signals, OS_EVENT_ANY | OS_EVENT_NOWAIT, 0, &events);
    return (int32_t)events;
}

osEvent osSignalWait(int32_t signals, uint32_t millisec)
{
    osEvent event = {0};
    U32 events = 0;
    U32 mask = (signals == 0) ? CMSIS_V1_VALID_SIGNAL_MASK : (U32)signals;
    U32 mode = ((signals == 0) ? OS_EVENT_ANY : OS_EVENT_ALL) | ((millisec == 0) ? OS_EVENT_NOWAIT : OS_EVENT_WAIT);

    if (OS_INT_ACTIVE) {
        event.status = osErrorISR;
        return event;
    }
    if (((U32)signals & ~CMSIS_V1_VALID_SIGNAL_MASK) != 0) {
        event.status = osErrorValue;
        return event;
    }
    if (PRT_EventRead(mask, mode, CmsisMsToTick(millisec), &events) == OS_OK) {
        if (events == 0) {
            event.status = osOK;
        } else {
            event.status = osEventSignal;
            event.value.signals = (int32_t)events;
        }
    } else {
        event.status = osEventTimeout;
    }
    return event;
}

osStatus osDelay(uint32_t millisec)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (millisec == 0) {
        return osOK;
    }
    return (PRT_TaskDelay(CmsisMsToTick(millisec)) == OS_OK) ? osEventTimeout : osErrorResource;
}

osEvent osWait(uint32_t millisec)
{
    osEvent event = {0};
    if (OS_INT_ACTIVE) {
        event.status = osErrorISR;
        return event;
    }
    if (millisec == 0) {
        event.status = osOK;
        return event;
    }
    event.status = osDelay(millisec);
    return event;
}

#endif
