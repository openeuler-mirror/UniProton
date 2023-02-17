/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-15
 * Description: os内部pthread功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"
#include "../../core/kernel/task/prt_task_internal.h"
#include "prt_err_external.h"

#define PTHREAD_TERMINATED  2
#define PTHREAD_EXITED      3

void (*destor[PTHREAD_KEYS_MAX])(void *) = {0};

static void OsPthreadNotifyParents(struct TagTskCb *tskCb)
{
    U32 count = tskCb->joinCount;

    while (count--) {
        PRT_SemPost(tskCb->joinableSem);
    }
}

static void OsPthreadRunDestructor(struct TagTskCb *self)
{
    int i;
    void *val;
    void (*destructor)(void *);

    for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if ((self->tsdUsed & (1U << (U32)i)) != 0) {
            val = self->tsd[i];
            destructor = destor[i];
            destructor(val);
        }
    }
}

void PRT_PthreadExit(void *retval)
{
    U32 ret;
    TskHandle task;
    uintptr_t intSave;
    struct TagTskCb *tskCb;

    intSave = OsIntLock();

    tskCb = RUNNING_TASK;
    while (tskCb->cancelBuf) {
        void (*f)(void *) = tskCb->cancelBuf->_routine;
        void *x = tskCb->cancelBuf->_arg;
        tskCb->cancelBuf = tskCb->cancelBuf->_previous;
        f(x);
    }

    task = tskCb->taskPid;
    /* thread is joinable and other threads are waitting */
    if (tskCb->state == PTHREAD_CREATE_JOINABLE && tskCb->joinCount > 0) {
        tskCb->retval = retval;
        tskCb->state = PTHREAD_EXITED;
        OsPthreadNotifyParents(tskCb);
    } else {
        tskCb->state = PTHREAD_TERMINATED;
        if (tskCb->joinableSem != 0) {
            PRT_SemDelete(tskCb->joinableSem);
        }
        OsPthreadRunDestructor(tskCb);
    }

    OsIntRestore(intSave);

    ret = PRT_TaskDelete(task);
    if (ret != OS_OK) {
        OsErrRecord(ret);
    }
}

static U32 OsPthreadCreatParaCheck(TskHandle *newthread, const pthread_attr_t *attrp,
    prt_pthread_startroutine routine, pthread_attr_t *attr)
{
    int ret;

    if (newthread == NULL || routine == NULL) {
        return EINVAL;
    }

    if (attrp != NULL) {
        if (attrp->is_initialized == PTHREAD_ATTR_UNINIT) {
            return EINVAL;
        }
        *attr = *attrp;
    } else {
        ret = pthread_attr_init(attr);
        if (ret != OS_OK) {
            OsErrRecord(ret);
        }
    }

    return OS_OK;
}

static void OsPthreadWrapper(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    void *ret;

    (void)param3;
    (void)param4;
    void *(*threadroutine)(void *) = (void *)param1;

    ret = threadroutine((void *)param2);
    PRT_PthreadExit(ret);
}

OS_SEC_ALW_INLINE INLINE U32 OsPthreadCreateRsrcInit(U32 taskId, pthread_attr_t *attr,
    struct TagTskCb *tskCb, uintptr_t **topStackOut, uintptr_t *curStackSize)
{
    U32 ret = OS_OK;
    uintptr_t *topStack = NULL;

    /* 创建任务线程 */
    if (g_taskNameAdd != NULL) {
        ret = g_taskNameAdd(taskId, "pthread");
        if (ret != OS_OK) {
            return ret;
        }
    }

    /* 查看用户是否配置了任务栈，如没有，则进行内存申请，并标记为系统配置，如有，则标记为用户配置。 */
    if (attr->stackaddr != 0) {
        topStack = (void *)(attr->stackaddr);
        tskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_USER;
    } else {
        topStack = OsTskMemAlloc(attr->stacksize);
        if (topStack == NULL) {
            ret = OS_ERRNO_TSK_NO_MEMORY;
        } else {
            tskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_SYS;
        }
    }
    *curStackSize = attr->stacksize;
    if (ret != OS_OK) {
        return ret;
    }

    *topStackOut = topStack;
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsPthreadCreateTcbInit(uintptr_t stackPtr, pthread_attr_t *attr,
    uintptr_t topStackAddr, uintptr_t curStackSize, struct TagTskCb *tskCb)
{
    /* Initialize the task's stack */
    tskCb->stackPointer = (void *)stackPtr;
    tskCb->topOfStack = topStackAddr;
    tskCb->stackSize = curStackSize;
    tskCb->taskSem = NULL;
    tskCb->priority = attr->schedparam.sched_priority;
    tskCb->taskEntry = OsPthreadWrapper;
#if defined(OS_OPTION_EVENT)
    tskCb->event = 0;
    tskCb->eventMask = 0;
#endif
    tskCb->lastErr = 0;
    tskCb->taskStatus = OS_TSK_SUSPEND | OS_TSK_INUSE;
    /* pthread init */
    tskCb->tsdUsed = 0;
    tskCb->state = attr->detachstate;
    tskCb->cancelState = PTHREAD_CANCEL_ENABLE;
    tskCb->cancelType = PTHREAD_CANCEL_DEFERRED;
    tskCb->cancelPending = 0;
	tskCb->cancelBuf = NULL;
    tskCb->retval = NULL;
    tskCb->joinCount = 0;
    tskCb->joinableSem = 0;
    tskCb->tsdUsed = 0;

    INIT_LIST_OBJECT(&tskCb->pendList);
    INIT_LIST_OBJECT(&tskCb->timerList);
}

int PRT_PthreadCreate(TskHandle *thread, const pthread_attr_t *attrp,
                       prt_pthread_startroutine routine, void *arg)
{
    U32 ret;
    U32 taskId;
    uintptr_t intSave;
    void *stackPtr = NULL;
    uintptr_t *topStack = NULL;
    uintptr_t curStackSize = 0;
    struct TagTskCb *tskCb = NULL;
    pthread_attr_t attr = {0};

    ret = OsPthreadCreatParaCheck(thread, attrp, routine, &attr);
    if (ret != OS_OK) {
        return ret;
    }

    intSave = OsIntLock();
    ret = OsTaskCreateChkAndGetTcb(&tskCb);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ENOMEM;
    }

    taskId = tskCb->taskPid;

    ret = OsPthreadCreateRsrcInit(taskId, &attr, tskCb, &topStack, &curStackSize);
    if (ret != OS_OK) {
        ListAdd(&tskCb->pendList, &g_tskCbFreeList);
        OsIntRestore(intSave);
        return ENOMEM;
    }
    OsTskStackInit(curStackSize, (uintptr_t)topStack);

    stackPtr = OsTskContextInit(taskId, curStackSize, topStack, (uintptr_t)OsTskEntry);

    OsPthreadCreateTcbInit((uintptr_t)stackPtr, &attr, (uintptr_t)topStack, curStackSize, tskCb);
    tskCb->args[OS_TSK_PARA_0] = (uintptr_t)routine;
    tskCb->args[OS_TSK_PARA_1] = (uintptr_t)arg;
    tskCb->args[OS_TSK_PARA_2] = 0;
    tskCb->args[OS_TSK_PARA_3] = 0;
    OsIntRestore(intSave);

    if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
        ret = PRT_SemCreate(0, &tskCb->joinableSem);
        if (ret != OS_OK) {
            return EAGAIN;
        }
    }

    ret = PRT_TaskResume(taskId);
    if (ret != OS_OK) {
        if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
            PRT_SemDelete(tskCb->joinableSem);
        }
        return EAGAIN;
    }
    *thread = taskId;

    return OS_OK;
}

void OsPthreadDestructor(void *arg)
{
    (void)arg;
}

int PRT_PthreadKeyCreate(pthread_key_t *key, void (*destructor)(void *))
{
    int i;

    for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if (destor[i] == NULL) {
            *key = (pthread_key_t)i;
            break;
        }
    }

    if (i == PTHREAD_KEYS_MAX) {
        return EAGAIN;
    }

    if (destructor != NULL) {
        destor[i] = destructor;
    } else {
        destor[i] = OsPthreadDestructor;
    }

    return OS_OK;
}

int PRT_PthreadKeyDelete(pthread_key_t key)
{
    U32 i;
    uintptr_t intSave;

    if ((U32)key >= PTHREAD_KEYS_MAX || destor[key] == NULL) {
        return EINVAL;
    }

    intSave = OsIntLock();
    for (i = 0; i < g_tskMaxNum; i++) {
        g_tskCbArray[i].tsdUsed = g_tskCbArray[i].tsdUsed & ~(1U << (U32)key);
        g_tskCbArray[i].tsd[key] = NULL;
    }
    destor[key] = NULL;
    OsIntRestore(intSave);

    return OS_OK;
}

int PRT_PthreadSetSpecific(pthread_key_t key, const void *value)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    if ((U32)key >= PTHREAD_KEYS_MAX || destor[key] == NULL) {
        return EINVAL;
    }
    tskCb->tsd[key] = (void *)(uintptr_t)value;
    tskCb->tsdUsed |= (1U << (U32)key);

    return OS_OK;
}

void *PRT_PthreadGetSpecific(pthread_key_t key)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    if ((U32)key >= PTHREAD_KEYS_MAX) {
        return NULL;
    }
    if ((tskCb->tsdUsed & (1U << (U32)key)) == 0) {
        return NULL;
    }

    return tskCb->tsd[key];
}

static void OsPthreadDoCleanupPush(struct _pthread_cleanup_context *cb)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    cb->_previous = tskCb->cancelBuf;
    tskCb->cancelBuf = cb;
}

static void OsPthreadDoCleanupPop(struct _pthread_cleanup_context *cb)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    tskCb->cancelBuf = cb->_previous;
}

void _pthread_cleanup_push(struct _pthread_cleanup_context *cb, void (*f)(void *), void *x)
{
    cb->_routine = f;
    cb->_arg = x;
    OsPthreadDoCleanupPush(cb);
}

void _pthread_cleanup_pop(struct _pthread_cleanup_context *cb, int run)
{
    OsPthreadDoCleanupPop(cb);
    if (run) cb->_routine(cb->_arg);
}

int PRT_PthreadJoin(TskHandle thread, void **status)
{
    struct TagTskCb *tskCb = RUNNING_TASK;
    uintptr_t intSave;
    U32 ret = 0;

    if (thread == tskCb->taskPid) {
        return EDEADLK;
    }
    if (CHECK_TSK_PID_OVERFLOW(thread)) {
        return ESRCH;
    }

    intSave = OsIntLock();

    tskCb = GET_TCB_HANDLE(thread);
    /* the target thread already exited */
    if (tskCb->taskStatus == OS_TSK_UNUSED) {
        OsIntRestore(intSave);
        return ret;
    }

    if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
        /* wait the target thread to finish */
        tskCb->joinCount++;
        OsIntRestore(intSave);
        ret = PRT_SemPend(tskCb->joinableSem, OS_WAIT_FOREVER);
        if (ret != OS_OK) {
            tskCb->joinCount--;
            return EDEADLK;
        }
        intSave = OsIntLock();
        tskCb->joinCount--;
    }

    if (tskCb->state == PTHREAD_EXITED) {
        if (status != NULL) {
            *status = tskCb->retval;
        }
        /* the last parent frees the resources */
        if (tskCb->joinCount == 0) {
            tskCb->state = PTHREAD_TERMINATED;
            if (tskCb->joinableSem != 0) {
                PRT_SemDelete(tskCb->joinableSem);
            }
            OsPthreadRunDestructor(tskCb);
        }
    } else if (tskCb->state == PTHREAD_CREATE_DETACHED) {
        ret = EINVAL;
    } else if (tskCb->state == PTHREAD_TERMINATED) {
        ret = 0;
    } else {
        ret = ESRCH;
    }
    OsIntRestore(intSave);

    return ret;
}

int PRT_PthreadDetach(TskHandle thread)
{
    struct TagTskCb *tskCb;
    uintptr_t intSave;
    U32 ret = 0;

    if (CHECK_TSK_PID_OVERFLOW(thread)) {
        return ESRCH;
    }

    tskCb = GET_TCB_HANDLE(thread);
    intSave = OsIntLock();

    switch (tskCb->state) {
        case PTHREAD_CREATE_JOINABLE:
            tskCb->state = PTHREAD_CREATE_DETACHED;
            OsPthreadNotifyParents(tskCb);
            break;
        case PTHREAD_EXITED:
            PRT_PthreadJoin(thread, NULL);
            break;
        case PTHREAD_TERMINATED:
            ret = ESRCH;
            break;
        default:
            ret = EINVAL;
            break;
    }

    OsIntRestore(intSave);

    return ret;
}

void PRT_PthreadTestCancel(void)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    if (tskCb->cancelState == PTHREAD_CANCEL_ENABLE && tskCb->cancelPending) {
        PRT_PthreadExit(PTHREAD_CANCELED);
    }
}

static int OsPthreadCancelDetachedHandle(struct TagTskCb *tskCb)
{
    U32 ret;

    tskCb->state = PTHREAD_TERMINATED;
    if (tskCb->joinableSem != 0) {
        ret = PRT_SemDelete(tskCb->joinableSem);
        if (ret != OS_OK) {
            return EINVAL;
        }
    }
    while (tskCb->cancelBuf) {
        void (*f)(void *) = tskCb->cancelBuf->_routine;
        void *x = tskCb->cancelBuf->_arg;
        tskCb->cancelBuf = tskCb->cancelBuf->_previous;
        f(x);
    }
    OsPthreadRunDestructor(tskCb);
    ret = PRT_TaskDelete(tskCb->taskPid);
    if (ret != OS_OK) {
        return EAGAIN;
    }

    return OS_OK;
}

static int OsPthreadCancelJoinableHandle(struct TagTskCb *tskCb)
{
    U32 ret;

    while (tskCb->cancelBuf) {
        void (*f)(void *) = tskCb->cancelBuf->_routine;
        void *x = tskCb->cancelBuf->_arg;
        tskCb->cancelBuf = tskCb->cancelBuf->_previous;
        f(x);
    }
    if (tskCb->joinCount == 0) {
        tskCb->state = PTHREAD_TERMINATED;
        if (tskCb->joinableSem != 0) {
            ret = PRT_SemDelete(tskCb->joinableSem);
            if (ret != OS_OK) {
                return EINVAL;
            }
        }
        OsPthreadRunDestructor(tskCb);
    } else {
        tskCb->state = PTHREAD_EXITED;
        while (tskCb->joinCount) {
            tskCb->joinCount--;
            ret = PRT_SemPost(tskCb->joinableSem);
            if (ret != OS_OK) {
                return EINVAL;
            }
        }
    }
    ret = PRT_TaskDelete(tskCb->taskPid);
    if (ret != OS_OK) {
        return EAGAIN;
    }

    return OS_OK;
}

int PRT_PthreadCancel(TskHandle thread)
{
    struct TagTskCb *tskCb;
    uintptr_t intSave;
    int ret = OS_OK;
    struct TagTskCb *curTskCb = RUNNING_TASK;

    if (CHECK_TSK_PID_OVERFLOW(thread)) {
        return ESRCH;
    }

    tskCb = GET_TCB_HANDLE(thread);
    intSave = OsIntLock();
    if (thread == curTskCb->taskPid) {
        if (tskCb->cancelState == PTHREAD_CANCEL_ENABLE && tskCb->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
            PRT_PthreadExit(PTHREAD_CANCELED);
        }
        OsIntRestore(intSave);
        return OS_OK;
    }

    tskCb->cancelPending = 1;
    if (tskCb->cancelType == PTHREAD_CANCEL_DEFERRED) {
        OsIntRestore(intSave);
        return OS_OK;
    }
    if (tskCb->cancelState == PTHREAD_CANCEL_ENABLE) {
        if (tskCb->state == PTHREAD_CREATE_DETACHED) {
            ret = OsPthreadCancelDetachedHandle(tskCb);
        } else if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
            ret = OsPthreadCancelJoinableHandle(tskCb);
        }
    }

    OsIntRestore(intSave);

    return ret;
}

int PRT_PthreadSetCancelState(int state, int *oldstate)
{
    struct TagTskCb *tskCb = RUNNING_TASK;
    uintptr_t intSave;

    /* currently, only supports ENABLE and DISABLE */
    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
        return EINVAL;
    }

    intSave = OsIntLock();
    if (oldstate != NULL) {
        *oldstate = tskCb->cancelState;
    }
    tskCb->cancelState = state;
    if (state == PTHREAD_CANCEL_ENABLE && tskCb->cancelPending) {
        PRT_PthreadExit(PTHREAD_CANCELED);
    }
    OsIntRestore(intSave);

    return OS_OK;
}

int PRT_PthreadSetCancelType(int type, int *oldType)
{
    struct TagTskCb *tskCb = RUNNING_TASK;
    uintptr_t intSave;
    
    if (type != PTHREAD_CANCEL_DEFERRED && type != PTHREAD_CANCEL_ASYNCHRONOUS) {
        return EINVAL;
    }

    intSave = OsIntLock();
    if (oldType != NULL) {
        *oldType = tskCb->cancelType;
    }
    tskCb->cancelType = type;
    if (type == PTHREAD_CANCEL_ASYNCHRONOUS) {
        PRT_PthreadTestCancel();
    }
    OsIntRestore(intSave);

    return OS_OK;
}
