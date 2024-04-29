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
 * Create: 2024-01-25
 * Description: 实时调度函数实现
 */
#include "prt_rt_internal.h"

OS_SEC_DATA struct TagScheduleClass g_osRtSingleSchedClass = {
    .osEnqueueTask = OsEnqueueTaskRtSingle,
    .osDequeueTask = OsDequeueTaskRtSingle,
    .osPutPrevTask = OsPutPrevTaskRtSingle,
    .osPickNextTask = OsPickNextTaskRtSingle,
    .osNextReadyTask = OsNextReadyRtTaskSingle,
};
/*
* 描述：查找优先级数组中最高优先级（越右，左移位数越少，优先级越高）
*/
OS_SEC_ALW_INLINE INLINE U32 OsFindHighestPri(const U32 *priArray, U32 size)
{
    U32 loop;
    for (loop = 0; loop < size; loop++) {
        if (priArray[loop] != 0) {
            return (OsGetRMB(priArray[loop]) + OS_GET_32BIT_ARRAY_BASE(loop));
        }
    }
    
    return OS_TSK_NUM_OF_PRIORITIES;
}

INIT_SEC_L4_TEXT struct TagScheduleClass *OsGetRtSingleSchedClass(void)
{
    return &g_osRtSingleSchedClass;
}

OS_SEC_L0_TEXT struct TagTskCb *OsNextReadyRtTaskSingle(struct TagOsRunQue *runQue)
{
    struct TagTskCb *task = NULL;
    struct TagListObject *readyList = NULL;
    struct RtActiveTskList *array = &runQue->rtRq.activeTsk;

    readyList = &array->readyList[runQue->currntPrio];
    task = GET_TCB_READY(OS_LIST_FIRST(readyList));

    return task;
}

OS_SEC_L0_TEXT void OsEnqueueTaskRtSingle(struct TagOsRunQue *runQue, struct TagTskCb *task, U32 flags)
{
    bool head = ((flags & OS_ENQUEUE_HEAD) != 0);
    // 入就绪优先级队列
    OsEnqueueReadyListOnly(task, head, task->priority, &runQue->rtRq);
    if (runQue->currntPrio > task->priority) {
        runQue->currntPrio = task->priority;
        runQue->needReschedule = TRUE;
    } else if (UNLIKELY(head && (runQue->currntPrio == task->priority))) {
        // 如果是插入到队头，则也需要重新出发调度
        runQue->needReschedule = TRUE;
    }
    return;
}

OS_SEC_L0_TEXT void OsDequeueTaskRtSingle(struct TagOsRunQue *runQue, struct TagTskCb *task, U32 flags)
{
    struct RtRq *rtRq = &runQue->rtRq;

    (void)flags;

    OsDequeueReadyListOnly(runQue,task); // CRG 暂无入队列头的场景
    // 出队的任务优先级不低于新的优先级，需要出发调度
    if(runQue->currntPrio >= task->priority) {
        runQue->currntPrio = OsFindHighestPri(&rtRq->activeTsk.readyPrioBit[0],
            sizeof(rtRq->activeTsk.readyPrioBit) / sizeof(rtRq->activeTsk.readyPrioBit[0]));
        runQue->needReschedule = TRUE;
    }
    return;
}

/*
 * 描述：RT_SINGLE处理切出任务
 * 备注：当前在RT_SINGLE的入口直接展开，不使用钩子进入，减少函数调用消耗
 */
OS_SEC_L0_TEXT void OsPutPrevTaskRtSingle(struct TagOsRunQue *runQue, struct TagTskCb *prevTskCB)
{
    (void)runQue;
    TSK_STATUS_CLEAR(prevTskCB, OS_TSK_RUNNING);
    return;
}

/*
 * 描述：RT_SINGLE主调度pick函数
 * 备注：NA
 */
OS_SEC_L0_TEXT struct TagTskCb *OsPickNextTaskRtSingle(struct TagOsRunQue *runQue)
{
    struct TagTskCb *task = NULL;

    // 上锁 直到中间可能的doublelock时解锁，或者schedule结束时解锁
    OsSplLock(&runQue->spinLock);
    runQue->needReschedule = FALSE;
    if (runQue->rqCoreId == runQue->tskCurr->coreID) {
        TSK_STATUS_CLEAR(runQue->tskCurr, OS_TSK_RUNNING);
    }

    task = OsNextReadyRtTaskSingle(runQue);
    runQue->tskCurr = task;
    TSK_STATUS_SET(task, OS_TSK_RUNNING);

    OsSplUnlock(&runQue->spinLock);

    return task;
}