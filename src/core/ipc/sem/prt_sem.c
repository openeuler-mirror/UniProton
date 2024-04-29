/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: 信号量模块
 */
#include "prt_sem_external.h"
#include "prt_asm_cpu_external.h"
#include "prt_task_sched_external.h"

/* 核内信号量最大个数 */
OS_SEC_BSS U16 g_maxSem;

#if defined(OS_OPTION_BIN_SEM)
OS_SEC_ALW_INLINE INLINE U32 OsSemPostErrorCheck(struct TagSemCb *semPosted, SemHandle semHandle)
{
    (void)semHandle;
    /* 检查信号量控制块是否UNUSED，排除大部分错误场景 */
    if (semPosted->semStat == OS_SEM_UNUSED) {
        return OS_ERRNO_SEM_INVALID;
    }
    if (GET_SEM_TYPE((semPosted)->semType) == SEM_TYPE_COUNT) {
        /* 释放计数型信号量且信号量计数大于最大计数 */
        if ((semPosted)->semCount >= OS_SEM_COUNT_MAX) {
            return OS_ERRNO_SEM_OVERFLOW;
        }
    } else if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
        if (OS_INT_ACTIVE) {
            return OS_ERRNO_SEM_MUTEX_POST_INTERR;
        }

        /* 如果不是 互斥信号量的持有任务来做post操作 */
        if (semPosted->semOwner != RUNNING_TASK->taskPid) {
            return OS_ERRNO_SEM_MUTEX_NOT_OWNER_POST;
        }
    }
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsSemPostBinMutex(struct TagSemCb *semPosted, struct TagTskCb *resumedTask)
{
    semPosted->semOwner = resumedTask->taskPid;
    if (semPosted->semType == SEM_TYPE_BIN) {
        ListDelete(&semPosted->semBList);
        ListTailAdd(&semPosted->semBList, &resumedTask->semBList);
    }
}
#else
OS_SEC_ALW_INLINE INLINE U32 OsSemPostErrorCheck(struct TagSemCb *semPosted, SemHandle semHandle)
{
    (void)semHandle;
    /* 检查信号量控制块是否UNUSED，排除大部分错误场景 */
    if (semPosted->semStat == OS_SEM_UNUSED) {
        return OS_ERRNO_SEM_INVALID;
    }

    /* post计数型信号量的错误场景, 释放计数型信号量且信号量计数大于最大计数 */
    if ((semPosted)->semCount >= OS_SEM_COUNT_MAX) {
        return OS_ERRNO_SEM_OVERFLOW;
    }

    return OS_OK;
}
#endif

/*
 * 描述：把当前运行任务挂接到信号量链表上
 */
OS_SEC_L0_TEXT void OsSemPendListPut(struct TagSemCb *semPended, U32 timeOut)
{
    struct TagTskCb *curTskCb = NULL;
    struct TagTskCb *runTsk = RUNNING_TASK;
    struct TagListObject *pendObj = &runTsk->pendList;
    OsSemIfPrioLock(semPended);

    OsTskReadyDel((struct TagTskCb *)runTsk);

    runTsk->taskSem = (void *)semPended;

    TSK_STATUS_SET(runTsk, OS_TSK_PEND);

#if defined(OS_OPTION_SEM_PRIOR)
    /* 根据唤醒方式挂接此链表，同优先级再按FIFO子顺序插入 */
    if (semPended->semMode == SEM_MODE_PRIOR) {
        LIST_FOR_EACH(curTskCb, &semPended->semList, struct TagTskCb, pendList) {
            if (curTskCb->priority > runTsk->priority) {
                ListTailAdd(pendObj, &curTskCb->pendList);
                goto TIMER_ADD;
            }
        }
    }
#endif
    /* 如果到这里，说明是FIFO方式；或者是优先级方式且挂接首个节点或者挂接尾节点 */
    ListTailAdd(pendObj, &semPended->semList);
TIMER_ADD:
    // timer超时链表添加
    if (timeOut != OS_WAIT_FOREVER) {
        /* 如果不是永久等待则将任务挂到计时器链表中，设置OS_TSK_TIMEOUT是为了判断是否等待超时 */
        TSK_STATUS_SET(runTsk, OS_TSK_TIMEOUT);

        OsTskTimerAdd((struct TagTskCb *)runTsk, timeOut);
    }

    OsSemIfPrioUnLock(semPended);
}

/*
 * 描述：从非空信号量链表上摘首个任务放入到ready队列
 */
OS_SEC_L0_TEXT struct TagTskCb *OsSemPendListGet(struct TagSemCb *semPended)
{
    struct TagTskCb *taskCb = GET_TCB_PEND(OS_LIST_FIRST(&(semPended->semList)));

    ListDelete(OS_LIST_FIRST(&(semPended->semList)));

    OsSpinLockTaskRq(taskCb);
    /* 如果阻塞的任务属于定时等待的任务时候，去掉其定时等待标志位，并将其从去除 */
    if (TSK_STATUS_TST(taskCb, OS_TSK_TIMEOUT)) {
        OS_TSK_DELAY_LOCKED_DETACH(taskCb);
        TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT);
    }

    /* 必须先去除 OS_TSK_TIMEOUT 态，再入队[睡眠时是先出ready队，再置OS_TSK_TIMEOUT态] */
    TSK_STATUS_CLEAR(taskCb, OS_TSK_PEND);
    taskCb->taskSem = NULL;
    /* 如果去除信号量阻塞位后，该任务不处于阻塞态则将该任务挂入就绪队列并触发任务调度 */
    if (!TSK_STATUS_TST(taskCb, OS_TSK_SUSPEND)) {
        OsTskReadyAddBgd(taskCb);
    }
    OsSpinUnlockTaskRq(taskCb);

#if defined(OS_OPTION_BIN_SEM)
    OsSemPostBinMutex(semPended, taskCb);
#endif
    return taskCb;
}

OS_SEC_L0_TEXT U32 OsSemPendParaCheck(U32 timeout)
{
    if (timeout == 0) {
        return OS_ERRNO_SEM_UNAVAILABLE;
    }

    if (OS_TASK_LOCK_DATA != 0) {
        return OS_ERRNO_SEM_PEND_IN_LOCK;
    }
    return OS_OK;
}

OS_SEC_L0_TEXT bool OsSemPendNotNeedSche(struct TagSemCb *semPended, struct TagTskCb *runTsk)
{
#if defined(OS_OPTION_SEM_RECUR_PV)
    if (GET_SEM_TYPE(semPended->semType) == SEM_TYPE_BIN && semPended->semOwner == runTsk->taskPid &&
        (GET_MUTEX_TYPE(semPended->semType) == PTHREAD_MUTEX_RECURSIVE)) {
        semPended->recurCount++;
        return TRUE;
    }
#endif

    if (semPended->semCount > 0) {
        semPended->semCount--;
        semPended->semOwner = runTsk->taskPid;
#if defined(OS_OPTION_BIN_SEM)
        /* 如果是互斥信号量，把持有的互斥信号量挂接起来 */
        if (GET_SEM_TYPE(semPended->semType) == SEM_TYPE_BIN) {
            ListTailAdd(&semPended->semBList, &runTsk->semBList);
        }
#endif
        return TRUE;
    }
    return FALSE;
}

/*
 * 描述：指定信号量的P操作
 */
OS_SEC_L0_TEXT U32 PRT_SemPend(SemHandle semHandle, U32 timeout)
{
    uintptr_t intSave;
    U32 ret;
    struct TagTskCb *runTsk = NULL;
    struct TagSemCb *semPended = NULL;

    if (semHandle >= (SemHandle)g_maxSem) {
        return OS_ERRNO_SEM_INVALID;
    }

    semPended = GET_SEM(semHandle);

    SEM_CB_IRQ_LOCK(semPended, intSave);

    if (semPended->semStat == OS_SEM_UNUSED) {
        SEM_CB_IRQ_UNLOCK(semPended, intSave);
        return OS_ERRNO_SEM_INVALID;
    }

    if (OS_INT_ACTIVE) {
        SEM_CB_IRQ_UNLOCK(semPended, intSave);
        return OS_ERRNO_SEM_PEND_INTERR;
    }

    runTsk = (struct TagTskCb *)RUNNING_TASK;

    if (OsSemPendNotNeedSche(semPended, runTsk) == TRUE) {
        SEM_CB_IRQ_UNLOCK(semPended, intSave);
        return OS_OK;
    }

    ret = OsSemPendParaCheck(timeout);
    if (ret != OS_OK) {
        SEM_CB_IRQ_UNLOCK(semPended, intSave);
        return ret;
    }
    /* 把当前任务挂接在信号量链表上 */
    OsSemPendListPut(semPended, timeout);

    SEM_CB_UNLOCK(semPended);
    if (timeout != OS_WAIT_FOREVER) {
        /* 触发任务调度 */
        OsTskSchedule();

        /* 判断是否是等待信号量超时 */
        if (TSK_STATUS_TST(runTsk, OS_TSK_TIMEOUT)) {

            struct TagOsRunQue *thisQue = OsSpinLockRunTaskRq();

            TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);

            OsSpinUnLockRunTaskRq(thisQue);
            OsIntRestore(intSave);
            return OS_ERRNO_SEM_TIMEOUT;
        }
    } 
    /* 恢复ps的快速切换 */
    OsTskScheduleFastPs(intSave);
    OsIntRestore(intSave);
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsSemPostSchePre(struct TagSemCb *semPosted)
{
    struct TagTskCb *resumedTask = NULL;

    resumedTask = OsSemPendListGet(semPosted);
    semPosted->semOwner = resumedTask->taskPid;
#if defined(OS_OPTION_BIN_SEM)
    /*
     * 如果释放的是互斥信号量，就从释放此互斥信号量任务的持有链表上摘除它，
     * 再把它挂接到新的持有它的任务的持有链表上；然后尝试降低任务的优先级
     */
    if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
        ListDelete(&semPosted->semBList);
        ListTailAdd(&semPosted->semBList, &resumedTask->semBList);
    }
#endif
}

/*
 * 描述：判断信号量post是否有效
 * 备注：以下情况表示post无效，返回TRUE: post互斥二进制信号量，若该信号量被嵌套pend或者已处于空闲状态
 */
OS_SEC_ALW_INLINE INLINE bool OsSemPostIsInvalid(struct TagSemCb *semPosted)
{
    if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
#if defined(OS_OPTION_SEM_RECUR_PV)
        if ((GET_MUTEX_TYPE(semPosted->semType) == PTHREAD_MUTEX_RECURSIVE) && semPosted->recurCount > 0) {
            semPosted->recurCount--;
            return TRUE;
        }
#endif
        /* 释放互斥二进制信号量且信号量已处于空闲状态 */
        if ((semPosted)->semCount == OS_SEM_FULL) {
            return TRUE;
        }
    }
    return FALSE;
}

/*
 * 描述：指定信号量的V操作
 */
OS_SEC_L0_TEXT U32 PRT_SemPost(SemHandle semHandle)
{
    U32 ret;
    uintptr_t intSave;
    struct TagSemCb *semPosted = NULL;

    if (semHandle >= (SemHandle)g_maxSem) {
        return OS_ERRNO_SEM_INVALID;
    }

    semPosted = GET_SEM(semHandle);
    SEM_CB_IRQ_LOCK(semPosted, intSave);

    ret = OsSemPostErrorCheck(semPosted, semHandle);
    if (ret != OS_OK) {
        SEM_CB_IRQ_UNLOCK(semPosted, intSave);
        return ret;
    }

    /* 信号量post无效，不需要调度 */
    if (OsSemPostIsInvalid(semPosted) == TRUE) {
        SEM_CB_IRQ_UNLOCK(semPosted, intSave);
        return OS_OK;
    }
    OsSemIfPrioLock(semPosted);

    /* 如果有任务阻塞在信号量上，就激活信号量阻塞队列上的首个任务 */
    if (!ListEmpty(&semPosted->semList)) {
        OsSemPostSchePre(semPosted);
        OsSemIfPrioUnLock(semPosted);
        SEM_CB_UNLOCK(semPosted);
        /* 相当于快速切换+中断恢复 */
        OsTskScheduleFastPs(intSave);
    } else {
        semPosted->semCount++;
        
#if defined(OS_OPTION_BIN_SEM)
        semPosted->semOwner = OS_INVALID_OWNER_ID;
        /* 如果释放的是互斥信号量，就从释放此互斥信号量任务的持有链表上摘除它 */
        if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
            ListDelete(&semPosted->semBList);
        }
#endif
        OsSemIfPrioUnLock(semPosted);
        SEM_CB_UNLOCK(semPosted);
    }

    OsIntRestore(intSave);
    return OS_OK;
}
