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
 * Create: 2024-01-27
 * Description: Task模块
 */
#ifndef PRT_SMP_TASK_INTERNAL_H
#define PRT_SMP_TASK_INTERNAL_H

#include "prt_task_external.h"
#include "prt_task_internal.h"

/* 判断任务是否在RQ上，在调用之前需要加RQ锁 */
#define OS_TSK_IS_ON_RQ(tsk)  ((tsk)->isOnRq)

/* 判断任务是否在RQ上，在调用之前需要加RQ锁 */
#define OS_TSK_IS_NOT_ON_RQ(tsk)  (!((tsk)->isOnRq))
#define OS_TSK_EN_QUE(runQue, tsk, flags) OsEnqueueTask((runQue), (tsk), (flags))
#define OS_TSK_EN_QUE_HEAD(runQue, tsk, flags) OsEnqueueTask((runQue), (tsk), (flags))
#define OS_TSK_DE_QUE(runQue, tsk, flags) OsDequeueTask((runQue), (tsk), (flags))

#define OS_TSK_RE_SCHED(task) OsReschedTask(task)

extern void OsTskDlyNearestTicksRefresh(struct TagOsTskSortedDelayList *tskDlyBase);
#define OS_TSK_DLY_NEAREST_TICK_REFRESH(tskDlyBase) OsTskDlyNearestTicksRefresh(tskDlyBase)

#define OS_SET_DLYBASE_AND_TSK_CORE(dlyBase, tsk)       \
    do {                                                \
        U32 thisCoreID = THIS_CORE();                   \
        (dlyBase) = CPU_TSK_DELAY_BASE(thisCoreID);     \
        (tsk)->timeCoreID = thisCoreID;                 \
    } while (0)

extern U32 OsTskSMPInit(void);
extern U32 OsIdleTskSMPCreate(void);
extern void OsSmpWakeUpSecondaryCore(void);
#if (OS_MAX_CORE_NUM > 1)
extern U32 OsTaskOperaBegin(struct TagTskCb *runTask, struct TagTskCb* tgtTask);
extern void OsTaskOperaEnd(struct TagTskCb *runTask, struct TagTskCb* tgtTask);
#else
OS_SEC_ALW_INLINE INLINE U32 OsTaskOperaBegin(struct TagTskCb *runTask, struct TagTskCb* tgtTask)
{
    (void)runTask;
    (void)tgtTask;
    return OS_OK;
}
OS_SEC_ALW_INLINE INLINE void OsTaskOperaEnd(struct TagTskCb *runTask, struct TagTskCb* tgtTask)
{
    (void)runTask;
    (void)tgtTask;
    return OS_OK;
}
#endif

OS_SEC_ALW_INLINE INLINE void OsTskSMPTcbInit(struct TskInitParam *initParam, struct TagTskCb *taskCB)
{
    OS_LIST_INIT(&taskCB->pushAbleList.nodeList);
    TSK_CORE_SET(taskCB, OsGetHwThreadId());
    taskCB->timeCoreID = taskCB->coreID;
    taskCB->isOnRq = FALSE;
    taskCB->taskOperating = OS_TSK_OP_FREE;
    taskCB->opBusy = 0;
    taskCB->pushAbleList.prio = initParam->taskPrio;
    taskCB->scheClass = RT_SINGLE_CLASS();
    taskCB->coreAllowedMask = (OS_CORE_MASK)OS_ALLCORES_MASK;
    taskCB->nrCoresAllowed = g_maxNumOfCores;
}

OS_SEC_ALW_INLINE INLINE struct TagTskCb *OsGetCurrentTcb(void)
{
    uintptr_t intSave = OsIntLock();
    struct TagTskCb *runTask = RUNNING_TASK;
    OsIntRestore(intSave);
    return runTask;
}
#endif