/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-26
 * Description: Task schedule implementation
 */
#include "prt_task_internal.h"
#if defined(OS_OPTION_TASK_YIELD)
OS_SEC_ALW_INLINE INLINE U32 OsTaskYieldProc(TskHandle nextTaskId, U32 taskPrio, TskHandle *yieldTo,
                                             struct TagTskCb *currTask,
                                             struct TagListObject *tskPriorRdyList)
{
    struct TagTskCb *nextTask = NULL;
    /* there is no task that user particularly wishes */
    /* therefore second task will become ready */
    if (nextTaskId == OS_TSK_NULL_ID) {
        if (yieldTo != NULL) {
            *yieldTo = (GET_TCB_PEND(OS_LIST_FIRST(&currTask->pendList)))->taskPid;
        }
        ListDelete(&currTask->pendList);
        ListTailAdd(&currTask->pendList, tskPriorRdyList);
    } else {
        if (yieldTo != NULL) {
            *yieldTo = nextTaskId;
        }

        nextTask = GET_TCB_HANDLE(nextTaskId);
        if ((nextTask->priority == taskPrio) && TSK_STATUS_TST(nextTask, OS_TSK_READY)) {
            ListDelete(&nextTask->pendList);
            ListAdd(&nextTask->pendList, tskPriorRdyList);
        } else { /* task is illegal */
            return OS_ERRNO_TSK_YIELD_INVALID_TASK;
        }
    }

    return OS_OK;
}

/*
 * 描述：调整指定优先级的任务调度顺序。调用者负责关中断
 */
static OS_SEC_L2_TEXT U32 OsTaskYield(TskPrior taskPrio, TskHandle nextTaskId, TskHandle *yieldTo, uintptr_t intSave)
{
    U32 ret;
    U32 tskCount = 0;
    struct TagTskCb *currTask = NULL;
    struct TagTskCb *runTask = NULL;
    struct TagListObject *tskPriorRdyList = NULL;
    struct TagListObject *currNode = NULL;
    runTask = RUNNING_TASK;

    OsSpinLockTaskRq(runTask);
    tskPriorRdyList = OS_TASK_GET_PRI_LIST(taskPrio);
    /* In case there are more then one ready tasks at */
    /* this priority, remove first task and add it */
    /* to the end of the queue */
    currTask = GET_TCB_PEND(OS_LIST_FIRST(tskPriorRdyList));

    for (currNode = tskPriorRdyList->next; currNode != tskPriorRdyList; currNode = currNode->next) {
        tskCount++;
    }

    if (tskCount > 1) {
        ret = OsTaskYieldProc(nextTaskId, taskPrio, yieldTo, currTask, tskPriorRdyList);
        if (ret != OS_OK) {
            OsSpinUnlockTaskRq(runTask);
            return ret;
        }

        if (TSK_STATUS_TST(currTask, OS_TSK_RUNNING)) {
            OsRescheduleCurr(THIS_RUNQ());
            OsSpinUnlockTaskRq(runTask);
            OsTskScheduleFast();
            return OS_OK;
        }
    } else { /* There is only one task or none */
        OsSpinUnlockTaskRq(runTask);
        return OS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK;
    }

    OsSpinUnlockTaskRq(runTask);
    return OS_OK;
}
#endif

/*
 * 描述：延迟当前运行任务的执行
 */
OS_SEC_L0_TEXT U32 PRT_TaskDelay(U32 tick)
{
    U32 ret;
    uintptr_t intSave;
    struct TagTskCb *runTask = NULL;

    intSave = OsIntLock();

    if (OS_INT_ACTIVE) {
        OS_REPORT_ERROR(OS_ERRNO_TSK_DELAY_IN_INT);
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_DELAY_IN_INT;
    }

    if (OS_TASK_LOCK_DATA != 0) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_DELAY_IN_LOCK;
    }
    runTask = RUNNING_TASK;

#if defined(OS_OPTION_TICK)
    if (tick > 0) {
        OsSpinLockTaskRq(runTask);
        OsTskReadyDel(runTask);
        TSK_STATUS_SET(runTask, OS_TSK_DELAY);
        OsTskTimerAdd(runTask, tick);
        OsSpinUnlockTaskRq(runTask);
        OsTskScheduleFastPs(intSave);
        OsIntRestore(intSave);

        return OS_OK;
    }
#else
    (void)tick;
#endif

#if defined(OS_OPTION_TASK_YIELD)
    ret = OsTaskYield(runTask->priority, OS_TSK_NULL_ID, NULL, intSave);
#else
    ret = OS_OK;
#endif
    OsIntRestore(intSave);

    return ret;
}

#if defined(OS_OPTION_TASK_YIELD)
OS_SEC_L2_TEXT U32 PRT_TaskYield(TskPrior taskPrio, TskHandle nextTask, TskHandle *yieldTo)
{
    uintptr_t intSave;
    U32 ret;

    if (taskPrio > OS_TSK_PRIORITY_LOWEST) {
        return OS_ERRNO_TSK_PRIOR_ERROR;
    }

    if ((nextTask != OS_TSK_NULL_ID) && (CHECK_TSK_PID_OVERFLOW(nextTask))) {
        return OS_ERRNO_BUILD_ID_INVALID;
    }

    intSave = OsIntLock();
    ret = OsTaskYield(taskPrio, nextTask, yieldTo, intSave);
    OsIntRestore(intSave);
    return ret;
}
#endif

OS_SEC_L0_TEXT void PRT_TaskLockNoIntLock(void)
{
    OS_TSK_LOCK();
}
/*
 * 描述：锁任务调度
 */
OS_SEC_L0_TEXT void PRT_TaskLock(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

    OS_TSK_LOCK();

    OsIntRestore(intSave);
}
OS_SEC_L0_TEXT uintptr_t PRT_TaskIrqLock(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

    OS_TSK_LOCK();
    
    return intSave;
}
/*
 * 描述：解锁锁任务调度
 */
OS_SEC_ALW_INLINE INLINE void OsTaskIrqUnlock(uintptr_t intSave)
{
    struct TagOsRunQue *rq = THIS_RUNQ();

    if(LIKELY(rq->uniTaskLock == 1)) {
        rq->uniTaskLock = 0;

        if (UNLIKELY(rq->needReschedule) && UNLIKELY((rq->uniFlag & OS_HWI_ACTIVE_MASK) == 0)) {
            if ((OS_DI_STATE_CHECK(intSave)) != 0) {
                SMP_MC_SCHEDULE_TRIGGER(THIS_CORE());
                return;
            } else {
                OsTskScheduleFast();
            }
        }
        OsIntRestore(intSave);
        return;
    }
    if(rq->uniTaskLock > 1) {
        rq->uniTaskLock--;
        OsIntRestore(intSave);
        return;
    }
    OS_REPORT_ERROR(OS_ERRNO_TSK_UNLOCK_NO_LOCK);
    OsIntRestore(intSave);
}
OS_SEC_L0_TEXT void PRT_TaskUnlock(void)
{
    uintptr_t intSave;
    intSave = OsIntLock();
    OsTaskIrqUnlock(intSave);
}
OS_SEC_L0_TEXT void PRT_TaskIrqUnlock(uintptr_t intSave)
{
    OsTaskIrqUnlock(intSave);
}