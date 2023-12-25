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
        OsTskReadyDel(currTask);  // 从当前就绪队列删除
        OsTskReadyAdd(currTask);  // 添加到就绪队列末尾
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
static OS_SEC_L2_TEXT U32 OsTaskYield(TskPrior taskPrio, TskHandle nextTaskId, TskHandle *yieldTo)
{
    U32 ret;
    U32 tskCount = 0;
    struct TagTskCb *currTask = NULL;
    struct TagListObject *tskPriorRdyList = NULL;
    struct TagListObject *currNode = NULL;

    tskPriorRdyList = &g_runQueue.readyList[taskPrio];
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
            return ret;
        }

        // 如果是当前的running任务
        if (TSK_STATUS_TST(currTask, OS_TSK_RUNNING)) {
            OsTskScheduleFast();

            return OS_OK;
        }
    } else { /* There is only one task or none */
        OS_REPORT_ERROR(OS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK);
        return OS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK;
    }

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
    if (UNI_FLAG == 0) {
        OsIntRestore(intSave);
        // 初始化过程中不允许切换
        return OS_ERRNO_TSK_DELAY_IN_INT;
    }

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
    if (tick > 0) {
        OsTskReadyDel(runTask);
        TSK_STATUS_SET(runTask, OS_TSK_DELAY);
        OsTskTimerAdd(runTask, tick);
        OsTskScheduleFastPs(intSave);
        OsIntRestore(intSave);

        return OS_OK;
    }

#if defined(OS_OPTION_TASK_YIELD)
    ret = OsTaskYield(runTask->priority, OS_TSK_NULL_ID, NULL);
#else
    ret = OS_OK;
#endif
    OsIntRestore(intSave);

    return ret;
}

/*
 * 描述：锁任务调度
 */
OS_SEC_L0_TEXT void PRT_TaskLock(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

    if (g_uniTaskLock == 0) {
        UNI_FLAG &= (~OS_FLG_TSK_REQ);
    }

    OS_TSK_LOCK();
    OsIntRestore(intSave);
}

/*
 * 描述：解锁锁任务调度
 */
OS_SEC_L0_TEXT void PRT_TaskUnlock(void)
{
    uintptr_t intSave = OsIntLock();

    if (g_uniTaskLock == 1) {
        // 参照osTskUnlock注释，热点函数特殊实现
        g_uniTaskLock = 0;
        if ((OS_FLG_BGD_ACTIVE & UNI_FLAG) != 0) {
            OsTskScheduleFastPs(intSave);
        }
        OsIntRestore(intSave);
        return;
    }
    // 冷分支
    if (g_uniTaskLock > 1) {
        g_uniTaskLock--;
        OsIntRestore(intSave);
        return;
    }
    // unlock时 已为0，说明lock unlock不配对，记录错误
    OS_REPORT_ERROR(OS_ERRNO_TSK_UNLOCK_NO_LOCK);
    OsIntRestore(intSave);
}
