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
#include "prt_task_external.h"
#include "prt_sem_external.h"
#include "prt_task_sched_external.h"

/*
 * 描述：获取指定任务的优先级
 */
OS_SEC_L4_TEXT U32 PRT_TaskGetPriority(TskHandle taskPid, TskPrior *taskPrio)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    if (taskPrio == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    taskCb = GET_TCB_HANDLE(taskPid);

    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);

        return OS_ERRNO_TSK_NOT_CREATED;
    }

    *taskPrio = taskCb->priority;

    OsIntRestore(intSave);

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsTaskPrioritySetCheck(TskHandle taskPid, TskPrior taskPrio)
{
    if (taskPrio > OS_TSK_PRIORITY_LOWEST) {
        return OS_ERRNO_TSK_PRIOR_ERROR;
    }

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }
    if (taskPid >= g_tskMaxNum) {

        return OS_ERRNO_TSK_OPERATE_IDLE;
    }
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsSemPrioBListLock(struct TagTskCb *taskCb)
{
#if defined(OS_OPTION_SEM_PRIO_INHERIT)
    if (!ListEmpty(&taskCb->semBList)) {
        OsSemPrioLock();
        return 1;
    }
#endif
    return 0;
}

OS_SEC_ALW_INLINE INLINE void OsSemPrioBListUnLock(U32 flag)
{
#if defined(OS_OPTION_SEM_PRIO_INHERIT)
    if (flag) {
        OsSemPrioUnLock();
    }
#else
    (void)flag;
#endif
}

/*
 * 描述：设置指定任务的优先级
 */
OS_SEC_L4_TEXT U32 PRT_TaskSetPriority(TskHandle taskPid, TskPrior taskPrio)
{
    U32 ret;
    bool isReady = FALSE;
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;
    U32 semPrioLockFlag;

    ret = OsTaskPrioritySetCheck(taskPid, taskPrio);
    if (ret != OS_OK) {
        return ret;
    }

    taskCb = GET_TCB_HANDLE(taskPid);
    intSave = OsIntLock();
    // 如果持有信号量互斥锁，避免遍历semBList时和semBList产生变化，需要锁semIfPrio
    semPrioLockFlag = OsSemPrioBListLock(taskCb);
    OsSpinLockTaskRq(taskCb);
    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        OsSpinUnlockTaskRq(taskCb);
        OsSemPrioBListUnLock(semPrioLockFlag);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

#if defined(OS_OPTION_SEM_PRIO_INHERIT)
    if (g_checkPrioritySet != NULL) {
        ret = g_checkPrioritySet(taskCb, taskPrio);
        if (ret != OS_OK) {
            OsSpinUnlockTaskRq(taskCb);
            OsSemPrioBListUnLock(semPrioLockFlag);
            OsIntRestore(intSave);
            return ret;
        }
    }
#endif

    isReady = (OS_TSK_READY & taskCb->taskStatus);

    /* delete the task & insert with right priority into ready queue */
    if (isReady) {
        OsTskReadyDel(taskCb);
        taskCb->priority = taskPrio;
        OsTskReadyAdd(taskCb);
    } else {
        taskCb->priority = taskPrio;
    }
    taskCb->origPriority = taskPrio;
    OsSpinUnlockTaskRq(taskCb);
    OsSemPrioBListUnLock(semPrioLockFlag);

    /* reschedule if ready changed */
    if (isReady) {
        OsTskSchedule();
    }

    OsIntRestore(intSave);
    return OS_OK;
}
