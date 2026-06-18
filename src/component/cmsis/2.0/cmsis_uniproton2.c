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
 * Description: CMSIS-RTOS v2 adapter subset for UniProton PRT APIs.
 */

#include "cmsis_uniproton.h"

#if (CMSIS_OS_VER == 2)

static const osVersion_t g_cmsisVersion = {20010016U, 20010016U};
static osKernelState_t g_kernelState = osKernelInactive;

typedef struct {
    U32 queueId;
    U32 msgSize;
    U32 msgCount;
} CmsisQueue;

typedef struct {
    SemHandle sem;
    U32 maxCount;
    const char *name;
} CmsisSemaphore;

typedef struct {
    SemHandle sem;
    const char *name;
} CmsisMutex;

typedef struct {
    TimerHandle handle;
    osTimerFunc_t func;
    void *argument;
    U32 running;
    osTimerType_t type;
    U32 intervalMs;
} CmsisTimer;

typedef struct {
    volatile U32 flags;
    U32 queueId;
    const char *name;
} CmsisEventFlags;

typedef struct {
    U32 blockSize;
    U32 blockCount;
    U32 used;
    const char *name;
} CmsisMemoryPool;

static osStatus_t CmsisSemStatus(U32 ret, osStatus_t unavailableStatus)
{
    if (ret == OS_OK) {
        return osOK;
    }
    if ((ret == OS_ERRNO_SEM_INVALID) || (ret == OS_ERRNO_SEM_PTR_NULL)) {
        return osErrorParameter;
    }
    if ((ret == OS_ERRNO_SEM_TIMEOUT) || (ret == OS_ERRNO_SEM_UNAVAILABLE)) {
        return unavailableStatus;
    }
    return osErrorResource;
}

static osStatus_t CmsisQueueStatus(U32 ret)
{
    if (ret == OS_OK) {
        return osOK;
    }
    if ((ret == OS_ERRNO_QUEUE_INVALID) || (ret == OS_ERRNO_QUEUE_NOT_CREATE) ||
        (ret == OS_ERRNO_QUEUE_PTR_NULL) || (ret == OS_ERRNO_QUEUE_SIZE_ZERO) ||
        (ret == OS_ERRNO_QUEUE_SIZE_TOO_BIG)) {
        return osErrorParameter;
    }
    if ((ret == OS_ERRNO_QUEUE_TIMEOUT) || (ret == OS_ERRNO_QUEUE_NO_SOURCE)) {
        return osErrorTimeout;
    }
    return osErrorResource;
}

static osThreadId_t CmsisHandleToThreadId(TskHandle taskId)
{
    return (osThreadId_t)(uintptr_t)(taskId + 1U);
}

static TskHandle CmsisThreadIdToHandle(osThreadId_t thread_id)
{
    return (TskHandle)((uintptr_t)thread_id - 1U);
}

osStatus_t osKernelInitialize(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (g_kernelState != osKernelInactive) {
        return osError;
    }
    if (CmsisKernelStarted()) {
        g_kernelState = osKernelRunning;
        return osOK;
    }
    g_kernelState = osKernelReady;
    return osOK;
}

osStatus_t osKernelGetInfo(osVersion_t *version, char *id_buf, uint32_t id_size)
{
    static const char id[] = "UniProton";

    if ((version == NULL) || (id_buf == NULL) || (id_size == 0)) {
        return osErrorParameter;
    }
    *version = g_cmsisVersion;
    (void)strncpy_s(id_buf, id_size, id, id_size - 1);
    id_buf[id_size - 1] = '\0';
    return osOK;
}

osKernelState_t osKernelGetState(void)
{
    if (!CmsisKernelStarted()) {
        if (g_kernelState == osKernelReady) {
            return osKernelReady;
        }
        return osKernelInactive;
    }
    if (CmsisKernelLocked()) {
        return osKernelLocked;
    }
    return osKernelRunning;
}

osStatus_t osKernelStart(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (g_kernelState == osKernelRunning) {
        return osOK;
    }
    if (g_kernelState != osKernelReady && !CmsisKernelStarted()) {
        return osError;
    }
    g_kernelState = osKernelRunning;
    return osOK;
}

int32_t osKernelLock(void)
{
    if (OS_INT_ACTIVE) {
        return (int32_t)osErrorISR;
    }
    if (!CmsisKernelStarted()) {
        return (int32_t)osError;
    }
    if (CmsisKernelLocked()) {
        return 1;
    }
    PRT_TaskLock();
    return 0;
}

int32_t osKernelUnlock(void)
{
    if (OS_INT_ACTIVE) {
        return (int32_t)osErrorISR;
    }
    if (!CmsisKernelStarted()) {
        return (int32_t)osError;
    }
    if (CmsisKernelLocked()) {
        PRT_TaskUnlock();
        if (CmsisKernelLocked()) {
            return (int32_t)osError;
        }
        return 1;
    }
    return 0;
}

int32_t osKernelRestoreLock(int32_t lock)
{
    if (OS_INT_ACTIVE) {
        return (int32_t)osErrorISR;
    }
    if (!CmsisKernelStarted()) {
        return (int32_t)osError;
    }
    if (lock == 0) {
        if (CmsisKernelLocked()) {
            PRT_TaskUnlock();
        }
        return 0;
    }
    if (lock == 1) {
        if (!CmsisKernelLocked()) {
            PRT_TaskLock();
        }
        return 1;
    }
    return (int32_t)osError;
}

uint32_t osKernelSuspend(void)
{
    if (OS_INT_ACTIVE || !CmsisKernelStarted()) {
        return 0;
    }
    return 0;
}

void osKernelResume(uint32_t sleep_ticks)
{
    (void)sleep_ticks;
}

uint64_t osKernelGetTickCount(void)
{
    return PRT_TickGetCount();
}

uint64_t osKernelGetTick2ms(void)
{
    return (uint64_t)CmsisTickToMs((U32)osKernelGetTickCount());
}

uint64_t osMs2Tick(uint64_t ms)
{
    return CmsisMsToTick((U32)ms);
}

uint32_t osKernelGetTickFreq(void)
{
    return OsSysGetTickPerSecond();
}

uint32_t osKernelGetSysTimerCount(void)
{
    return (uint32_t)PRT_ClkGetCycleCount64();
}

uint32_t osKernelGetSysTimerFreq(void)
{
    return OsSysGetClock();
}

static void CmsisThreadEntry(uintptr_t arg0, uintptr_t arg1, uintptr_t arg2, uintptr_t arg3)
{
    osThreadFunc_t func = (osThreadFunc_t)(uintptr_t)arg0;
    (void)arg2;
    (void)arg3;
    func((void *)arg1);
}

osThreadId_t osThreadNew(osThreadFunc_t func, void *argument, const osThreadAttr_t *attr)
{
    TskHandle taskId;
    U32 stackSize;
    U16 prio;
    const char *name;

    if ((func == NULL) || OS_INT_ACTIVE) {
        return NULL;
    }
    if (attr != NULL) {
        if (!CmsisPrioIsValid((int32_t)attr->priority)) {
            return NULL;
        }
        prio = CmsisPrioToPrt((int32_t)attr->priority);
        name = (attr->name == NULL) ? "cmsisTask" : attr->name;
        stackSize = attr->stack_size;
    } else {
        prio = CmsisPrioToPrt((int32_t)osPriorityNormal);
        name = "cmsisTask";
        stackSize = 0;
    }
    if (CmsisCreateTask(&taskId, CmsisThreadEntry, (uintptr_t)(void *)func, (uintptr_t)argument, name, stackSize, prio) != OS_OK) {
        return NULL;
    }
    return CmsisHandleToThreadId(taskId);
}

const char *osThreadGetName(osThreadId_t thread_id)
{
    char *name = NULL;
    if (thread_id == NULL) {
        return NULL;
    }
    return (PRT_TaskGetName(CmsisThreadIdToHandle(thread_id), &name) == OS_OK) ? name : NULL;
}

osThreadId_t osThreadGetId(void)
{
    TskHandle taskId;
    return (PRT_TaskSelf(&taskId) == OS_OK) ? CmsisHandleToThreadId(taskId) : NULL;
}

osThreadState_t osThreadGetState(osThreadId_t thread_id)
{
    TskStatus status;
    if ((thread_id == NULL) || OS_INT_ACTIVE) {
        return osThreadError;
    }
    status = PRT_TaskGetStatus(CmsisThreadIdToHandle(thread_id));
    if (status == (TskStatus)OS_INVALID) {
        return osThreadError;
    }
    if ((status & OS_TSK_RUNNING) != 0) {
        return osThreadRunning;
    }
    if ((status & OS_TSK_READY) != 0) {
        return osThreadReady;
    }
    if ((status & (OS_TSK_DELAY | OS_TSK_PEND | OS_TSK_SUSPEND | OS_TSK_EVENT_PEND | OS_TSK_QUEUE_PEND)) != 0) {
        return osThreadBlocked;
    }
    if (status == OS_TSK_UNUSED) {
        return osThreadInactive;
    }
    return osThreadError;
}

uint32_t osThreadGetStackSize(osThreadId_t thread_id)
{
    struct TskInfo info;
    if ((thread_id == NULL) || OS_INT_ACTIVE) {
        return 0;
    }
    return (PRT_TaskGetInfo(CmsisThreadIdToHandle(thread_id), &info) == OS_OK) ? info.stackSize : 0;
}

uint32_t osThreadGetStackSpace(osThreadId_t thread_id)
{
    struct TskInfo info;
    if ((thread_id == NULL) || OS_INT_ACTIVE) {
        return 0;
    }
    if (PRT_TaskGetInfo(CmsisThreadIdToHandle(thread_id), &info) != OS_OK) {
        return 0;
    }
    if (info.peakUsed >= info.stackSize) {
        return 0;
    }
    return info.stackSize - info.peakUsed;
}

osStatus_t osThreadSetPriority(osThreadId_t thread_id, osPriority_t priority)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (!CmsisPrioIsValid((int32_t)priority)) {
        return osErrorParameter;
    }
    return (PRT_TaskSetPriority(CmsisThreadIdToHandle(thread_id), CmsisPrioToPrt((int32_t)priority)) == OS_OK) ?
        osOK : osErrorResource;
}

osPriority_t osThreadGetPriority(osThreadId_t thread_id)
{
    TskPrior prio;
    if ((thread_id == NULL) || OS_INT_ACTIVE) {
        return osPriorityError;
    }
    if (PRT_TaskGetPriority(CmsisThreadIdToHandle(thread_id), &prio) != OS_OK) {
        return osPriorityError;
    }
    return (CmsisPrioFromPrt(prio) < 0) ? osPriorityError : (osPriority_t)CmsisPrioFromPrt(prio);
}

osStatus_t osThreadYield(void)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskDelay(0) == OS_OK) ? osOK : osError;
}

osStatus_t osThreadSuspend(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskSuspend(CmsisThreadIdToHandle(thread_id)) == OS_OK) ? osOK : osErrorResource;
}

osStatus_t osThreadResume(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskResume(CmsisThreadIdToHandle(thread_id)) == OS_OK) ? osOK : osErrorResource;
}

osStatus_t osThreadDetach(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return osOK;
}

osStatus_t osThreadJoin(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (PRT_TaskJoin(CmsisThreadIdToHandle(thread_id)) == 0) {
        return osOK;
    }
    return osErrorResource;
}

void osThreadExit(void)
{
    TskHandle taskId;
    if (PRT_TaskSelf(&taskId) == OS_OK) {
        (void)PRT_TaskDelete(taskId);
    }
    for (;;) {
    }
}

osStatus_t osThreadTerminate(osThreadId_t thread_id)
{
    if (thread_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskDelete(CmsisThreadIdToHandle(thread_id)) == OS_OK) ? osOK : osErrorResource;
}

uint32_t osThreadGetCount(void)
{
    U32 count = 0;
    U32 index;
    struct TagTskCb *taskCb;

    for (index = 0; index < OS_MAX_TCB_NUM; index++) {
        taskCb = GET_TCB_HANDLE_BY_TCBID(index + g_tskBaseId);
        if (!TSK_IS_UNUSED(taskCb)) {
            count++;
        }
    }
    return count;
}

uint32_t osThreadEnumerate(osThreadId_t *thread_array, uint32_t array_items)
{
    U32 count = 0;
    U32 index;
    struct TagTskCb *taskCb;

    if ((thread_array == NULL) || (array_items == 0)) {
        return 0;
    }
    for (index = 0; index < OS_MAX_TCB_NUM; index++) {
        if (count >= array_items) {
            break;
        }
        taskCb = GET_TCB_HANDLE_BY_TCBID(index + g_tskBaseId);
        if (!TSK_IS_UNUSED(taskCb)) {
            thread_array[count] = CmsisHandleToThreadId(taskCb->taskPid);
            count++;
        }
    }
    return count;
}

uint32_t osThreadFlagsSet(osThreadId_t thread_id, uint32_t flags)
{
    TskHandle taskId;
    struct TagTskCb *taskCb;
    U32 oldFlags;

    if (thread_id == NULL) {
        return osFlagsErrorParameter;
    }
    if ((flags == 0) || ((flags & osFlagsError) != 0)) {
        return osFlagsErrorParameter;
    }
    taskId = CmsisThreadIdToHandle(thread_id);
    taskCb = GET_TCB_HANDLE(taskId);
    if ((taskCb == NULL) || TSK_IS_UNUSED(taskCb)) {
        return osFlagsErrorResource;
    }
#if defined(OS_OPTION_EVENT)
    oldFlags = taskCb->event;
#else
    oldFlags = 0;
#endif
    if (PRT_EventWrite(taskId, flags) != OS_OK) {
        return osFlagsErrorResource;
    }
    return oldFlags | flags;
}

uint32_t osThreadFlagsClear(uint32_t flags)
{
    TskHandle taskId;
    struct TagTskCb *taskCb;
    U32 oldFlags;

    if (OS_INT_ACTIVE) {
        return osFlagsErrorUnknown;
    }
    if ((flags == 0) || ((flags & osFlagsError) != 0)) {
        return osFlagsErrorParameter;
    }
    if (PRT_TaskSelf(&taskId) != OS_OK) {
        return osFlagsErrorUnknown;
    }
    taskCb = GET_TCB_HANDLE(taskId);
    if ((taskCb == NULL) || TSK_IS_UNUSED(taskCb)) {
        return osFlagsErrorUnknown;
    }
#if defined(OS_OPTION_EVENT)
    oldFlags = taskCb->event;
    taskCb->event &= ~flags;
#else
    oldFlags = 0;
#endif
    return oldFlags;
}

uint32_t osThreadFlagsGet(void)
{
    TskHandle taskId;
    struct TagTskCb *taskCb;

    if (OS_INT_ACTIVE) {
        return osFlagsErrorUnknown;
    }
    if (PRT_TaskSelf(&taskId) != OS_OK) {
        return osFlagsErrorUnknown;
    }
    taskCb = GET_TCB_HANDLE(taskId);
    if ((taskCb == NULL) || TSK_IS_UNUSED(taskCb)) {
        return osFlagsErrorUnknown;
    }
#if defined(OS_OPTION_EVENT)
    return taskCb->event;
#else
    return 0;
#endif
}

uint32_t osThreadFlagsWait(uint32_t flags, uint32_t options, uint32_t timeout)
{
    U32 events = 0;
    U32 mode = ((options & osFlagsWaitAll) ? OS_EVENT_ALL : OS_EVENT_ANY) | (timeout == 0 ? OS_EVENT_NOWAIT : OS_EVENT_WAIT);

    if (OS_INT_ACTIVE) {
        return osFlagsErrorUnknown;
    }
    if (options > (osFlagsWaitAny | osFlagsWaitAll | osFlagsNoClear)) {
        return osFlagsErrorParameter;
    }
    if ((flags == 0) || ((flags & osFlagsError) != 0)) {
        return osFlagsErrorParameter;
    }
    if (PRT_EventRead(flags, mode, timeout, &events) == OS_OK) {
        if ((options & osFlagsNoClear) != 0) {
            (void)PRT_EventWrite(CmsisThreadIdToHandle(osThreadGetId()), events);
        }
        return events;
    }
    return (timeout == 0) ? osFlagsErrorResource : osFlagsErrorTimeout;
}

osStatus_t osDelay(uint32_t ticks)
{
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TaskDelay(ticks) == OS_OK) ? osOK : osError;
}

osStatus_t osDelayUntil(uint64_t ticks)
{
    U64 now = PRT_TickGetCount();
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (ticks <= now) ? osError : osDelay((uint32_t)(ticks - now));
}

static void CmsisTimerCallback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    CmsisTimer *timer = (CmsisTimer *)(uintptr_t)arg1;
    (void)tmrHandle;
    (void)arg2;
    (void)arg3;
    (void)arg4;
    if ((timer != NULL) && (timer->func != NULL)) {
        timer->func(timer->argument);
    }
}

osTimerId_t osTimerNew(osTimerFunc_t func, osTimerType_t type, void *argument, const osTimerAttr_t *attr)
{
    struct TimerCreatePara para = {0};
    CmsisTimer *timer;
    (void)attr;

    if ((func == NULL) || OS_INT_ACTIVE || ((type != osTimerOnce) && (type != osTimerPeriodic))) {
        return NULL;
    }

    timer = (CmsisTimer *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisTimer));
    if (timer == NULL) {
        return NULL;
    }
    timer->func = func;
    timer->argument = argument;
    timer->running = 0;
    timer->type = type;
    timer->intervalMs = CmsisTickToMs(1);

    para.type = OS_TIMER_SOFTWARE;
    para.mode = (type == osTimerPeriodic) ? OS_TIMER_LOOP : OS_TIMER_ONCE;
    para.interval = timer->intervalMs;
    para.callBackFunc = CmsisTimerCallback;
    para.arg1 = (U32)(uintptr_t)timer;

    if (PRT_TimerCreate(&para, &timer->handle) != OS_OK) {
        (void)PRT_MemFree(0, timer);
        return NULL;
    }
    return (osTimerId_t)timer;
}

const char *osTimerGetName(osTimerId_t timer_id)
{
    (void)timer_id;
    return NULL;
}

osStatus_t osTimerStart(osTimerId_t timer_id, uint32_t ticks)
{
    CmsisTimer *timer = (CmsisTimer *)timer_id;
    struct TimerCreatePara para = {0};
    U32 intervalMs = CmsisTickToMs(ticks);
    struct SwTmrInfo info;

    if (timer == NULL) {
        return osErrorParameter;
    }
    if (ticks == 0) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (PRT_SwTmrInfoGet(timer->handle, &info) == OS_OK) {
        if (info.state == (U8)OS_TIMER_RUNNING) {
            (void)PRT_TimerStop(0, timer->handle);
        }
    }
    if (timer->intervalMs != intervalMs) {
        (void)PRT_TimerDelete(0, timer->handle);
        para.type = OS_TIMER_SOFTWARE;
        para.mode = (timer->type == osTimerPeriodic) ? OS_TIMER_LOOP : OS_TIMER_ONCE;
        para.interval = intervalMs;
        para.callBackFunc = CmsisTimerCallback;
        para.arg1 = (U32)(uintptr_t)timer;
        if (PRT_TimerCreate(&para, &timer->handle) != OS_OK) {
            return osErrorResource;
        }
        timer->intervalMs = intervalMs;
    }
    if (PRT_TimerStart(0, timer->handle) != OS_OK) {
        return osErrorResource;
    }
    return osOK;
}

osStatus_t osTimerStop(osTimerId_t timer_id)
{
    CmsisTimer *timer = (CmsisTimer *)timer_id;
    if (timer == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_TimerStop(0, timer->handle) == OS_OK) ? osOK : osErrorResource;
}

uint32_t osTimerIsRunning(osTimerId_t timer_id)
{
    CmsisTimer *timer = (CmsisTimer *)timer_id;
    struct SwTmrInfo info;

    if ((timer == NULL) || OS_INT_ACTIVE) {
        return 0;
    }
    if (PRT_SwTmrInfoGet(timer->handle, &info) != OS_OK) {
        return 0;
    }
    return (info.state == (U8)OS_TIMER_RUNNING) ? 1 : 0;
}

osStatus_t osTimerDelete(osTimerId_t timer_id)
{
    CmsisTimer *timer = (CmsisTimer *)timer_id;
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

osEventFlagsId_t osEventFlagsNew(const osEventFlagsAttr_t *attr)
{
    CmsisEventFlags *ef;

    if (OS_INT_ACTIVE) {
        return NULL;
    }
    ef = (CmsisEventFlags *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisEventFlags));
    if (ef == NULL) {
        return NULL;
    }
    ef->flags = 0;
    ef->name = (attr == NULL) ? NULL : attr->name;
    if (PRT_QueueCreate(1, sizeof(U32), &ef->queueId) != OS_OK) {
        (void)PRT_MemFree(0, ef);
        return NULL;
    }
    return (osEventFlagsId_t)ef;
}

const char *osEventFlagsGetName(osEventFlagsId_t ef_id)
{
    CmsisEventFlags *ef = (CmsisEventFlags *)ef_id;
    return (ef == NULL) ? NULL : ef->name;
}

uint32_t osEventFlagsSet(osEventFlagsId_t ef_id, uint32_t flags)
{
    CmsisEventFlags *ef = (CmsisEventFlags *)ef_id;
    U32 dummy = 0;
    U32 oldFlags;

    if ((ef == NULL) || (flags == 0) || ((flags & osFlagsError) != 0)) {
        return osFlagsErrorParameter;
    }
    oldFlags = ef->flags;
    ef->flags |= flags;
    (void)PRT_QueueWrite(ef->queueId, &dummy, sizeof(dummy), 0, OS_QUEUE_NORMAL);
    return oldFlags | flags;
}

uint32_t osEventFlagsClear(osEventFlagsId_t ef_id, uint32_t flags)
{
    CmsisEventFlags *ef = (CmsisEventFlags *)ef_id;
    U32 oldFlags;
    uintptr_t intSave;

    if ((ef == NULL) || ((flags & osFlagsError) != 0)) {
        return osFlagsErrorParameter;
    }
    intSave = OsIntLock();
    oldFlags = ef->flags;
    ef->flags &= ~flags;
    OsIntRestore(intSave);
    return oldFlags;
}

uint32_t osEventFlagsGet(osEventFlagsId_t ef_id)
{
    CmsisEventFlags *ef = (CmsisEventFlags *)ef_id;
    U32 currentFlags;
    uintptr_t intSave;

    if (ef == NULL) {
        return osFlagsErrorParameter;
    }
    intSave = OsIntLock();
    currentFlags = ef->flags;
    OsIntRestore(intSave);
    return currentFlags;
}

static uint32_t CmsisEventFlagsCheck(CmsisEventFlags *ef, uint32_t flags, uint32_t options)
{
    U32 current = ef->flags;
    if ((options & osFlagsWaitAll) != 0) {
        if ((current & flags) == flags) {
            return current;
        }
    } else {
        if ((current & flags) != 0) {
            return current;
        }
    }
    return 0;
}

uint32_t osEventFlagsWait(osEventFlagsId_t ef_id, uint32_t flags, uint32_t options, uint32_t timeout)
{
    CmsisEventFlags *ef = (CmsisEventFlags *)ef_id;
    U32 result;
    U32 dummy;
    U32 len;
    U32 remaining = timeout;
    U64 startTick;

    if ((ef == NULL) || (flags == 0) || ((flags & osFlagsError) != 0)) {
        return osFlagsErrorParameter;
    }
    if (OS_INT_ACTIVE && (timeout != 0)) {
        return osFlagsErrorParameter;
    }
    if (options > (osFlagsWaitAny | osFlagsWaitAll | osFlagsNoClear)) {
        return osFlagsErrorParameter;
    }

    result = CmsisEventFlagsCheck(ef, flags, options);
    if (result != 0) {
        if ((options & osFlagsNoClear) == 0) {
            ef->flags &= ~flags;
        }
        return result;
    }

    if (timeout == 0) {
        return osFlagsErrorResource;
    }

    startTick = (U32)PRT_TickGetCount();
    for (;;) {
        len = sizeof(dummy);
        (void)PRT_QueueRead(ef->queueId, &dummy, &len, remaining);
        result = CmsisEventFlagsCheck(ef, flags, options);
        if (result != 0) {
            if ((options & osFlagsNoClear) == 0) {
                ef->flags &= ~flags;
            }
            return result;
        }
        if (remaining != osWaitForever) {
            U32 elapsed = (U32)PRT_TickGetCount() - startTick;
            if (elapsed >= timeout) {
                return osFlagsErrorTimeout;
            }
            remaining = timeout - elapsed;
        }
    }
}

osStatus_t osEventFlagsDelete(osEventFlagsId_t ef_id)
{
    CmsisEventFlags *ef = (CmsisEventFlags *)ef_id;
    if (ef == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    (void)PRT_QueueDelete(ef->queueId);
    (void)PRT_MemFree(0, ef);
    return osOK;
}

osMutexId_t osMutexNew(const osMutexAttr_t *attr)
{
    CmsisMutex *mutex;

    if (OS_INT_ACTIVE) {
        return NULL;
    }
    mutex = (CmsisMutex *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisMutex));
    if (mutex == NULL) {
        return NULL;
    }
#if defined(OS_OPTION_BIN_SEM)
    if (PRT_SemMutexCreate(&mutex->sem) != OS_OK) {
        (void)PRT_MemFree(0, mutex);
        return NULL;
    }
    mutex->name = (attr == NULL) ? NULL : attr->name;
    return (osMutexId_t)mutex;
#else
    (void)attr;
    (void)PRT_MemFree(0, mutex);
    return NULL;
#endif
}

const char *osMutexGetName(osMutexId_t mutex_id)
{
    CmsisMutex *mutex = (CmsisMutex *)mutex_id;
    return (mutex == NULL) ? NULL : mutex->name;
}

osStatus_t osMutexAcquire(osMutexId_t mutex_id, uint32_t timeout)
{
    CmsisMutex *mutex = (CmsisMutex *)mutex_id;
    U32 ret;

    if (mutex == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    ret = PRT_SemPend(mutex->sem, timeout);
    return CmsisSemStatus(ret, osErrorTimeout);
}

osStatus_t osMutexRelease(osMutexId_t mutex_id)
{
    CmsisMutex *mutex = (CmsisMutex *)mutex_id;
    if (mutex == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    return (PRT_SemPost(mutex->sem) == OS_OK) ? osOK : osErrorResource;
}

osThreadId_t osMutexGetOwner(osMutexId_t mutex_id)
{
    CmsisMutex *mutex = (CmsisMutex *)mutex_id;
    struct SemInfo semInfo;
    if ((mutex == NULL) || OS_INT_ACTIVE || (PRT_SemGetInfo(mutex->sem, &semInfo) != OS_OK) ||
        (semInfo.owner == OS_INVALID_OWNER_ID)) {
        return NULL;
    }
    return CmsisHandleToThreadId((TskHandle)semInfo.owner);
}

osStatus_t osMutexDelete(osMutexId_t mutex_id)
{
    CmsisMutex *mutex = (CmsisMutex *)mutex_id;
    if (mutex == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (PRT_SemDelete(mutex->sem) != OS_OK) {
        return osErrorResource;
    }
    (void)PRT_MemFree(0, mutex);
    return osOK;
}

osSemaphoreId_t osSemaphoreNew(uint32_t max_count, uint32_t initial_count, const osSemaphoreAttr_t *attr)
{
    CmsisSemaphore *sem;

    if ((max_count == 0) || (initial_count > max_count) || (max_count > OS_SEM_COUNT_MAX) || OS_INT_ACTIVE) {
        return NULL;
    }
    sem = (CmsisSemaphore *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisSemaphore));
    if (sem == NULL) {
        return NULL;
    }
    if (PRT_SemCreate(initial_count, &sem->sem) != OS_OK) {
        (void)PRT_MemFree(0, sem);
        return NULL;
    }
    sem->maxCount = max_count;
    sem->name = (attr == NULL) ? NULL : attr->name;
    return (osSemaphoreId_t)sem;
}

const char *osSemaphoreGetName(osSemaphoreId_t semaphore_id)
{
    CmsisSemaphore *sem = (CmsisSemaphore *)semaphore_id;
    return (sem == NULL) ? NULL : sem->name;
}

osStatus_t osSemaphoreAcquire(osSemaphoreId_t semaphore_id, uint32_t timeout)
{
    CmsisSemaphore *sem = (CmsisSemaphore *)semaphore_id;
    U32 ret;

    if ((sem == NULL) || (OS_INT_ACTIVE && (timeout != 0))) {
        return osErrorParameter;
    }
    ret = PRT_SemPend(sem->sem, timeout);
    return CmsisSemStatus(ret, osErrorTimeout);
}

osStatus_t osSemaphoreRelease(osSemaphoreId_t semaphore_id)
{
    CmsisSemaphore *sem = (CmsisSemaphore *)semaphore_id;
    U32 count;

    if (sem == NULL) {
        return osErrorParameter;
    }
    if (PRT_SemGetCount(sem->sem, &count) != OS_OK) {
        return osErrorParameter;
    }
    if (count >= sem->maxCount) {
        return osErrorResource;
    }
    return CmsisSemStatus(PRT_SemPost(sem->sem), osErrorResource);
}

uint32_t osSemaphoreGetCount(osSemaphoreId_t semaphore_id)
{
    CmsisSemaphore *sem = (CmsisSemaphore *)semaphore_id;
    U32 count = 0;
    if (sem != NULL) {
        (void)PRT_SemGetCount(sem->sem, &count);
    }
    return count;
}

osStatus_t osSemaphoreDelete(osSemaphoreId_t semaphore_id)
{
    CmsisSemaphore *sem = (CmsisSemaphore *)semaphore_id;
    osStatus_t status;

    if (sem == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    status = CmsisSemStatus(PRT_SemDelete(sem->sem), osErrorResource);
    if (status == osOK) {
        (void)PRT_MemFree(0, sem);
    }
    return status;
}

osMemoryPoolId_t osMemoryPoolNew(uint32_t block_count, uint32_t block_size, const osMemoryPoolAttr_t *attr)
{
    CmsisMemoryPool *pool;

    if ((block_count == 0) || (block_size == 0) || OS_INT_ACTIVE) {
        return NULL;
    }
    pool = (CmsisMemoryPool *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisMemoryPool));
    if (pool == NULL) {
        return NULL;
    }
    pool->blockSize = block_size;
    pool->blockCount = block_count;
    pool->used = 0;
    pool->name = (attr == NULL) ? NULL : attr->name;
    return (osMemoryPoolId_t)pool;
}

const char *osMemoryPoolGetName(osMemoryPoolId_t mp_id)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    return (pool == NULL) ? NULL : pool->name;
}

void *osMemoryPoolAlloc(osMemoryPoolId_t mp_id, uint32_t timeout)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    void *block;

    if (pool == NULL) {
        return NULL;
    }
    if (OS_INT_ACTIVE && (timeout != 0)) {
        return NULL;
    }
    if (pool->used >= pool->blockCount) {
        return NULL;
    }
    block = PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, pool->blockSize);
    if (block != NULL) {
        pool->used++;
    }
    return block;
}

osStatus_t osMemoryPoolFree(osMemoryPoolId_t mp_id, void *block)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;

    if ((mp_id == NULL) || (block == NULL)) {
        return osErrorParameter;
    }
    if (PRT_MemFree(0, block) != OS_OK) {
        return osErrorResource;
    }
    if (pool->used > 0) {
        pool->used--;
    }
    return osOK;
}

uint32_t osMemoryPoolGetCapacity(osMemoryPoolId_t mp_id)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    return (pool == NULL) ? 0 : pool->blockCount;
}

uint32_t osMemoryPoolGetBlockSize(osMemoryPoolId_t mp_id)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    return (pool == NULL) ? 0 : pool->blockSize;
}

uint32_t osMemoryPoolGetCount(osMemoryPoolId_t mp_id)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    return (pool == NULL) ? 0 : pool->used;
}

uint32_t osMemoryPoolGetSpace(osMemoryPoolId_t mp_id)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    return ((pool == NULL) || (pool->used > pool->blockCount)) ? 0 : pool->blockCount - pool->used;
}

osStatus_t osMemoryPoolDelete(osMemoryPoolId_t mp_id)
{
    CmsisMemoryPool *pool = (CmsisMemoryPool *)mp_id;
    if (mp_id == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    (void)PRT_MemFree(0, pool);
    return osOK;
}

osMessageQueueId_t osMessageQueueNew(uint32_t msg_count, uint32_t msg_size, const osMessageQueueAttr_t *attr)
{
    CmsisQueue *queue;
    (void)attr;

    if ((msg_count == 0) || (msg_size == 0) || OS_INT_ACTIVE) {
        return NULL;
    }
    queue = (CmsisQueue *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(CmsisQueue));
    if (queue == NULL) {
        return NULL;
    }
    if (PRT_QueueCreate((U16)msg_count, (U16)msg_size, &queue->queueId) != OS_OK) {
        (void)PRT_MemFree(0, queue);
        return NULL;
    }
    queue->msgSize = msg_size;
    queue->msgCount = msg_count;
    return (osMessageQueueId_t)queue;
}

const char *osMessageQueueGetName(osMessageQueueId_t mq_id)
{
    (void)mq_id;
    return NULL;
}

osStatus_t osMessageQueuePut(osMessageQueueId_t mq_id, const void *msg_ptr, uint8_t msg_prio, uint32_t timeout)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    (void)msg_prio;
    if ((queue == NULL) || (msg_ptr == NULL) || (OS_INT_ACTIVE && (timeout != 0))) {
        return osErrorParameter;
    }
    return CmsisQueueStatus(PRT_QueueWrite(queue->queueId, (void *)msg_ptr, queue->msgSize, timeout, OS_QUEUE_NORMAL));
}

osStatus_t osMessageQueueGet(osMessageQueueId_t mq_id, void *msg_ptr, uint8_t *msg_prio, uint32_t timeout)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    U32 len;
    (void)msg_prio;
    if ((queue == NULL) || (msg_ptr == NULL) || (OS_INT_ACTIVE && (timeout != 0))) {
        return osErrorParameter;
    }
    len = queue->msgSize;
    return CmsisQueueStatus(PRT_QueueRead(queue->queueId, msg_ptr, &len, timeout));
}

uint32_t osMessageQueueGetCapacity(osMessageQueueId_t mq_id)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    return (queue == NULL) ? 0 : queue->msgCount;
}

uint32_t osMessageQueueGetMsgSize(osMessageQueueId_t mq_id)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    return (queue == NULL) ? 0 : queue->msgSize;
}

uint32_t osMessageQueueGetCount(osMessageQueueId_t mq_id)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    U32 num = 0;
    if (queue != NULL) {
        (void)PRT_QueueGetNodeNum(queue->queueId, OS_QUEUE_PID_ALL, &num);
    }
    return num;
}

uint32_t osMessageQueueGetSpace(osMessageQueueId_t mq_id)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    U32 used = osMessageQueueGetCount(mq_id);
    return ((queue == NULL) || (used > queue->msgCount)) ? 0 : queue->msgCount - used;
}

osStatus_t osMessageQueueReset(osMessageQueueId_t mq_id)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    U8 discard[32];
    U32 len;

    if (queue == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    do {
        len = (queue->msgSize > sizeof(discard)) ? sizeof(discard) : queue->msgSize;
    } while (PRT_QueueRead(queue->queueId, discard, &len, 0) == OS_OK);
    return osOK;
}

osStatus_t osMessageQueueDelete(osMessageQueueId_t mq_id)
{
    CmsisQueue *queue = (CmsisQueue *)mq_id;
    if (queue == NULL) {
        return osErrorParameter;
    }
    if (OS_INT_ACTIVE) {
        return osErrorISR;
    }
    if (PRT_QueueDelete(queue->queueId) != OS_OK) {
        return osErrorResource;
    }
    (void)PRT_MemFree(0, queue);
    return osOK;
}

#endif
