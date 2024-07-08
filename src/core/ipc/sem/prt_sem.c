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

#if defined(OS_OPTION_SEM_PRIO_INHERIT)
/* 遍历任务持有的互斥信号量，获取阻塞中任务的最高优先级，需要semPrio锁保护 */
OS_SEC_ALW_INLINE INLINE void OsGetPendTskMaxPriority(struct TagTskCb *taskCb, TskPrior *maxPriority)
{
    TskPrior curMaxPrior = *maxPriority;
    struct TagSemCb *curSem = NULL;
    struct TagTskCb *pendTask = NULL;

    LIST_FOR_EACH(curSem, &taskCb->semBList, struct TagSemCb, semBList) {
        if (!ListEmpty(&curSem->semList)) {
            pendTask = GET_TCB_PEND(OS_LIST_FIRST(&(curSem->semList)));
            if (pendTask->priority < curMaxPrior) {
                curMaxPrior = pendTask->priority;
            }
        }
    }
    *maxPriority = curMaxPrior;
    return;
}

/*
 * 描述：判断任务是否可以重设优先级，PRT_TaskSetPriority中调用此函数。
 */
OS_SEC_L4_TEXT U32 OsCheckPrioritySet(struct TagTskCb *taskCb, TskPrior taskPrio)
{
    struct TagSemCb *curSem = NULL;
    TskPrior maxPriority = taskCb->origPriority;

    if (taskCb->origPriority != taskCb->priority) {
        return OS_ERRNO_TSK_PRIORITY_INHERIT;
    }

    curSem = taskCb->taskPend;
    if (TSK_STATUS_TST(taskCb, OS_TSK_PEND) && curSem != NULL) {
        if (GET_SEM_TYPE(curSem->semType) == SEM_TYPE_BIN) {
            return OS_ERRNO_TSK_PEND_MUTEX;
        }
        if (curSem->semMode == SEM_MODE_PRIOR) {
            // 只有开启优先级继承时才有该限制，保证唤醒的任务在等待队列中优先级最高，如此不需要触发优先级继承
            return OS_ERRNO_TSK_PEND_PRIOR;
        }
    }

    /* 未持有互斥信号量 */
    if (ListEmpty(&taskCb->semBList)) {
        return OS_OK;
    }

    /* 遍历持有的互斥信号量，获取pend任务的最高优先级，此处需要semIfPrior锁保护 */
    OsGetPendTskMaxPriority(taskCb, &maxPriority);

    if (taskPrio > maxPriority) {
        return OS_ERRNO_TSK_PRIOR_LOWER_THAN_PENDTSK;
    }

    return OS_OK;
}

/*
 * 描述：按照任务新设置的优先级，调整阻塞于信号量的任务优先级队列
 */
OS_SEC_ALW_INLINE INLINE void OsResortSemList(struct TagTskCb *taskCb)
{  
    /* 调整信号量优先级队列，外部需要锁OsSemIfPrioLock，taskrq */
    struct TagTskCb *curTskCb = NULL;
    struct TagSemCb *semPended = (struct TagSemCb *)taskCb->taskPend;

    ListDelete(&taskCb->pendList);

    /* 遍历链表，找到合适的位置插入 */
    LIST_FOR_EACH(curTskCb, &semPended->semList, struct TagTskCb, pendList) {
        /* 找到一个优先级低于目标任务的任务 */
        if (taskCb->priority < curTskCb->priority) {
            /* 插入到该任务前面 */
            ListTailAdd(&taskCb->pendList, &curTskCb->pendList);
            return;
        }
    }

    /* 优先级低于或等于队列中所有其他任务，加在整个队列尾部 */
    ListTailAdd(&taskCb->pendList, &semPended->semList);
}

/*
 * 描述：当前任务因为互斥信号量阻塞，需要升高持有互斥信号量的任务的优先级，外部受到semPrio锁保护
 */
OS_SEC_L0_TEXT void OsPriorityInherit(struct TagSemCb *semPended, struct TagTskCb *runTsk)
{
    struct TagTskCb *semOwnerTask;
    struct TagSemCb *recurSem;
    TskPrior newPriority = runTsk->priority;
    
    if (GET_SEM_TYPE(semPended->semType) != SEM_TYPE_BIN) {
        return;
    }

    semOwnerTask = GET_TCB_HANDLE(semPended->semOwner);

    OsSpinLockTaskRq(semOwnerTask);
    while (semOwnerTask->priority > newPriority) {
        /* 信号量持有任务处于就绪状态，调整就绪队列后退出 */
        if (TSK_STATUS_TST(semOwnerTask, OS_TSK_READY)) {
            OsTskReadyDel(semOwnerTask);
            semOwnerTask->priority = newPriority;
            /* 添加到就绪链表同优先级尾部 */
            OsTskReadyAdd(semOwnerTask);
            OsSpinUnlockTaskRq(semOwnerTask);
            return;
        }
        semOwnerTask->priority = newPriority;
        /* 信号量持有任务不处于信号量阻塞状态与就绪状态，直接退出 */
        if (!TSK_STATUS_TST(semOwnerTask, OS_TSK_PEND)) {
            OsSpinUnlockTaskRq(semOwnerTask);
            return;
        }
        /* 信号量持有任务也处于阻塞状态，获取所阻塞的信号量 */
        recurSem = (struct TagSemCb *)semOwnerTask->taskPend;
        /* 如果是优先级唤醒模式先重排信号量的阻塞优先级队列 */
        if (recurSem->semMode == SEM_MODE_PRIOR) {
            OsResortSemList(semOwnerTask);
        }
        OsSpinUnlockTaskRq(semOwnerTask);
        /* 所阻塞的信号量不为互斥型，直接退出 */
        if (GET_SEM_TYPE(recurSem->semType) != SEM_TYPE_BIN) {
            return;
        }
        /* 所阻塞的信号量类型为互斥型，多级优先级继承，提升拥有该互斥信号量的任务优先级 */
        semOwnerTask = GET_TCB_HANDLE(recurSem->semOwner);
        OsSpinLockTaskRq(semOwnerTask);
    }
    OsSpinUnlockTaskRq(semOwnerTask);
}

/*
 * 描述：任务释放互斥信号量时尝试恢复原本优先级，需要semPrio锁保护
 */
OS_SEC_L0_TEXT bool OsPriorityRestore(void)
{
    struct TagSemCb *curSem = NULL;
    struct TagTskCb *taskCb = NULL;
    struct TagTskCb *runTsk = RUNNING_TASK;
    TskPrior maxPriority = runTsk->origPriority;

    if (runTsk->priority == runTsk->origPriority) {
        return FALSE;
    }

    OsSpinLockTaskRq(runTsk);

    /* 遍历当前任务持有的互斥信号量，获取pend任务的最高优先级 */
    OsGetPendTskMaxPriority(runTsk, &maxPriority);

    /* 最高优先级与任务当前优先级相等，不需要调整 */
    if (maxPriority == runTsk->priority) {
        OsSpinUnlockTaskRq(runTsk);
        return FALSE;
    }

    /* 优先级降到最高优先，最高优先级至少为原始优先级 */
    OsTskReadyDel(runTsk);
    runTsk->priority = maxPriority;
    if (!TSK_STATUS_TST(runTsk, OS_TSK_SUSPEND_READY_BLOCK)) {
        // 中间放过锁，这里可能已经被置suspend, readAdd前必须有能否readyAdd的判断
        OsTskReadyAddBgd(runTsk);
    }
    OsSpinUnlockTaskRq(runTsk);
    return TRUE;
}
#endif

/*
 * 描述：把当前运行任务挂接到信号量链表上，外部受semPrio锁保护
 */
OS_SEC_L0_TEXT void OsSemPendListPut(struct TagSemCb *semPended, U32 timeOut)
{
    struct TagTskCb *curTskCb = NULL;
    struct TagTskCb *runTsk = RUNNING_TASK;
    struct TagListObject *pendObj = &runTsk->pendList;
    struct TagOsRunQue *runQue;

    runQue = OsSpinLockRunTaskRq();
    OsTskReadyDel((struct TagTskCb *)runTsk);

    runTsk->taskPend = (void *)semPended;

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

    OsSpinUnLockRunTaskRq(runQue);

#if defined(OS_OPTION_SEM_PRIO_INHERIT)
    /* 优先级继承，仅二进制信号量支持 */
    OsPriorityInherit(semPended, runTsk);
#endif
}

/*
 * 描述：从非空信号量链表上摘首个任务放入到ready队列
 */
OS_SEC_L0_TEXT void OsSemPendListGet(struct TagSemCb *semPended)
{
    /* 任务阻塞链表上的第一个任务/优先级最高的任务 */
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
    taskCb->taskPend = NULL;
    /* 如果去除信号量阻塞位后，该任务不处于阻塞态则将该任务挂入就绪队列并触发任务调度 */
    if (!TSK_STATUS_TST(taskCb, OS_TSK_SUSPEND)) {
        OsTskReadyAddBgd(taskCb);
    }
    OsSpinUnlockTaskRq(taskCb);

    semPended->semOwner = taskCb->taskPid;
#if defined(OS_OPTION_BIN_SEM)
    /*
     * 如果释放的是互斥信号量，就从释放此互斥信号量任务的持有链表上摘除它，
     * 再把它挂接到新的持有它的任务的持有链表上；然后尝试降低任务的优先级
     */
    if (GET_SEM_TYPE(semPended->semType) == SEM_TYPE_BIN) {
        // 此处需要受到 semIfPrio 锁保护
        ListDelete(&semPended->semBList);
        ListTailAdd(&semPended->semBList, &taskCb->semBList);  
#if defined(OS_OPTION_SEM_PRIO_INHERIT)
        (void)OsPriorityRestore();
#endif
    }
#endif
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
        GET_MUTEX_TYPE(semPended->semType) == SEM_MUTEX_TYPE_RECUR) {
        semPended->recurCount++;
        return TRUE;
    }
#endif

    if (semPended->semCount > 0) {
        semPended->semCount--;
        semPended->semOwner = runTsk->taskPid;
#if defined(OS_OPTION_BIN_SEM)
        /* 如果是互斥信号量，把持有的互斥信号量挂接起来，此处需要受semPrio锁保护 */
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
    struct TagOsRunQue *runQue = NULL;

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
    OsSemIfPrioLock(semPended);
    if (OsSemPendNotNeedSche(semPended, runTsk) == TRUE) {
        OsSemIfPrioUnLock(semPended);
        SEM_CB_IRQ_UNLOCK(semPended, intSave);
        return OS_OK;
    }

    ret = OsSemPendParaCheck(timeout);
    if (ret != OS_OK) {
        OsSemIfPrioUnLock(semPended);
        SEM_CB_IRQ_UNLOCK(semPended, intSave);
        return ret;
    }
    /* 把当前任务挂接在信号量链表上 */
    OsSemPendListPut(semPended, timeout);

    OsSemIfPrioUnLock(semPended);
    SEM_CB_UNLOCK(semPended);
    if (timeout != OS_WAIT_FOREVER) {
        /* 触发任务调度 */
        OsTskSchedule();

        runQue = OsSpinLockRunTaskRq();
        /* 判断是否是等待信号量超时 */
        if (TSK_STATUS_TST(runTsk, OS_TSK_TIMEOUT)) {
            TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);
            OsSpinUnLockRunTaskRq(runQue);
            OsIntRestore(intSave);
            return OS_ERRNO_SEM_TIMEOUT;
        }
        OsSpinUnLockRunTaskRq(runQue);
    } 
    /* 恢复ps的快速切换 */
    OsTskScheduleFastPs(intSave);
    OsIntRestore(intSave);
    return OS_OK;
}

/*
 * 描述：判断信号量post是否有效
 * 备注：以下情况表示post无效，返回TRUE: post互斥二进制信号量，若该信号量被嵌套pend或者已处于空闲状态
 */
OS_SEC_ALW_INLINE INLINE bool OsSemPostIsInvalid(struct TagSemCb *semPosted)
{
    if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
#if defined(OS_OPTION_SEM_RECUR_PV)
        if (GET_MUTEX_TYPE(semPosted->semType) == SEM_MUTEX_TYPE_RECUR && semPosted->recurCount > 0) {
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
        OsSemPendListGet(semPosted);
        OsSemIfPrioUnLock(semPosted);
        SEM_CB_UNLOCK(semPosted);
        /* 相当于快速切换+中断恢复 */
        OsTskScheduleFastPs(intSave);
    } else {
        semPosted->semCount++;
        
        semPosted->semOwner = OS_INVALID_OWNER_ID;
#if defined(OS_OPTION_BIN_SEM)
        /* 如果释放的是互斥信号量，就从释放此互斥信号量任务的持有链表上摘除它 */
        if (GET_SEM_TYPE(semPosted->semType) == SEM_TYPE_BIN) {
            ListDelete(&semPosted->semBList);
#if defined(OS_OPTION_SEM_PRIO_INHERIT)
            /* 尝试降低当前任务优先级 */
            if (OsPriorityRestore()) {
                /* 优先级发生变化，触发任务调度 */
                OsSemIfPrioUnLock(semPosted);
                SEM_CB_UNLOCK(semPosted);
                OsTskSchedule();
                OsIntRestore(intSave);
                return OS_OK;
            }
#endif
        }
#endif
        OsSemIfPrioUnLock(semPosted);
        SEM_CB_UNLOCK(semPosted);
    }

    OsIntRestore(intSave);
    return OS_OK;
}
