/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: pthread_create 相关接口实现
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "pthread.h"
#include "prt_posix_internal.h"
#if defined(OS_OPTION_SMP)
#include "../../../core/kernel/task/smp/prt_task_internal.h"
#else
#include "../../../core/kernel/task/amp/prt_task_internal.h"
#endif
#include "prt_err_external.h"
#include "prt_signal_external.h"
#if defined(OS_OPTION_POSIX_LOCALE)
#include "prt_posix_ext.h"
#endif
static U32 OsPthreadCreatParaCheck(TskHandle *newthread, const pthread_attr_t *attrp,
    prt_pthread_startroutine routine, pthread_attr_t *attr)
{
    if (newthread == NULL || routine == NULL) {
        return EINVAL;
    }

    if (attrp != NULL) {
        *attr = *attrp;
    } else {
        int ret = pthread_getattr_default_np(attr);
        if (ret != OS_OK) {
            OsErrRecord(ret);
        }
    }

    return OS_OK;
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

static void OsPthreadWrapper(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    void *ret;

    (void)param3;
    (void)param4;
    void *(*threadroutine)(void *) = (void *)param1;

    ret = threadroutine((void *)param2);
    PRT_PthreadExit(ret);
}

OS_SEC_ALW_INLINE INLINE void OsPthreadCreateTcbInit(uintptr_t stackPtr, pthread_attr_t *attr,
    uintptr_t topStackAddr, uintptr_t curStackSize, struct TagTskCb *tskCb)
{
    /* Initialize the task's stack */
    tskCb->stackPointer = (void *)stackPtr;
    tskCb->topOfStack = topStackAddr;
    tskCb->stackSize = curStackSize;
    tskCb->taskPend = NULL;
    tskCb->priority = attr->schedparam.sched_priority;
    tskCb->origPriority = tskCb->priority;
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

#if defined(OS_OPTION_RR_SCHED)
    if (attr->schedpolicy == SCHED_RR) {
        tskCb->policy = OS_TSK_SCHED_RR;
    } else {
        tskCb->policy = OS_TSK_SCHED_FIFO;
    }
    tskCb->startTime = 0;
    tskCb->timeSlice = g_timeSliceCycle;
#if defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
    tskCb->irqUsedTime = 0;
#endif
#endif

    INIT_LIST_OBJECT(&tskCb->semBList);
    INIT_LIST_OBJECT(&tskCb->pendList);
    INIT_LIST_OBJECT(&tskCb->timerList);
#if defined(OS_OPTION_POSIX_SIGNAL)
    tskCb->sigMask = 0;
    tskCb->sigWaitMask = 0;
    tskCb->sigPending = 0;
    tskCb->itimer = 0;
    INIT_LIST_OBJECT(&tskCb->sigInfoList);
    OsInitSigVectors(tskCb);
#endif
#if defined(OS_OPTION_POSIX_LOCALE)
    tskCb->locale = (locale_t)libc_global_locale;
#endif
#if defined(OS_OPTION_SMP)
    OS_LIST_INIT(&tskCb->pushAbleList.nodeList);
    TSK_CORE_SET(tskCb, OsGetHwThreadId());
    tskCb->timeCoreID = tskCb->coreID;
    tskCb->isOnRq = FALSE;
    tskCb->taskOperating = OS_TSK_OP_FREE;
    tskCb->opBusy = 0;
    tskCb->pushAbleList.prio = attr->schedparam.sched_priority;
    tskCb->scheClass = RT_SINGLE_CLASS();
    tskCb->coreAllowedMask = (OS_CORE_MASK)OS_ALLCORES_MASK;
    tskCb->nrCoresAllowed = g_maxNumOfCores;
#endif
}

int __pthread_create(pthread_t *newthread, const pthread_attr_t *attr, void *(*threadroutine)(void *), void *arg)
{
    TskHandle *thread = (TskHandle *)newthread;
    U32 ret;
    TskHandle taskId;
    uintptr_t intSave;
    void *stackPtr = NULL;
    uintptr_t *topStack = NULL;
    uintptr_t curStackSize = 0;
    struct TagTskCb *tskCb = NULL;
    pthread_attr_t attrp = {0};

    ret = OsPthreadCreatParaCheck(thread, attr, threadroutine, &attrp);
    if (ret != OS_OK) {
        return ret;
    }

    intSave = OsIntLock();
#if defined(OS_OPTION_SMP)
    ret = OsTaskCreateChkAndGetTcb(&tskCb, FALSE);
#else
    ret = OsTaskCreateChkAndGetTcb(&tskCb);  
#endif
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ENOMEM;
    }

    taskId = tskCb->taskPid;

    ret = OsPthreadCreateRsrcInit(taskId, &attrp, tskCb, &topStack, &curStackSize);
    if (ret != OS_OK) {
        ListAdd(&tskCb->pendList, &g_tskCbFreeList);
        OsIntRestore(intSave);
        return ENOMEM;
    }
    OsTskStackInit(curStackSize, (uintptr_t)topStack);

    stackPtr = OsTskContextInit(taskId, curStackSize, topStack, (uintptr_t)OsTskEntry);

    OsPthreadCreateTcbInit((uintptr_t)stackPtr, &attrp, (uintptr_t)topStack, curStackSize, tskCb);
    tskCb->args[OS_TSK_PARA_0] = (uintptr_t)threadroutine;
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
    *newthread = (pthread_t)taskId;

    return OS_OK;
}
weak_alias(__pthread_create, pthread_create);