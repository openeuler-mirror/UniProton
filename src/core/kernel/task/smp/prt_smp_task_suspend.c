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
#include "prt_task_internal.h"
#include "prt_smp_task_internal.h"

/*
 * 描述：task para check
 * 输入：taskPID --- 任务PID
 * 输出：NULL
 * 返回：OS_OK返回成功
 */
OS_SEC_ALW_INLINE INLINE U32 TaskParaCheck(TskHandle taskPID)
{
    if (CHECK_TSK_PID_OVERFLOW(taskPID)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    if (TSK_GET_INDEX(taskPID) >= g_tskMaxNum) {
        return OS_ERRNO_TSK_OPERATE_IDLE;
    }

    return OS_OK;
}
#if defined(OS_OPTION_TASK_SUSPEND)
OS_SEC_L2_TEXT U32 OsTaskSuspendPre(struct TagTskCb *runTask, struct TagTskCb *taskCB, uintptr_t *intSave)
{
    U32 ret;
    ret = OsTaskOperaBegin(runTask, taskCB);
    if (ret != OS_OK) {
        return ret;
    }

    ret = OsTryLockTaskOperating(OS_TSK_OP_SUSPENDING, taskCB, intSave);
    if (ret != OS_OK) {

        OsTaskOperaEnd(runTask, taskCB);
        return ret;
    }
    /* if task is suspended than return */
    if (TSK_STATUS_TST(taskCB, OS_TSK_SUSPEND)) {
        OS_TSK_OP_CLR(taskCB, OS_TSK_OP_SUSPENDING);
        OsSpinUnlockTaskRq(taskCB);
        OsTaskOperaEnd(runTask, taskCB);
        OsIntRestore(*intSave);
        return OS_ERRNO_TSK_ALREADY_SUSPENDED;
    }

    if ((taskCB == runTask) && (OS_TASK_LOCK_DATA != 0)) {
        OS_TSK_OP_CLR(taskCB, OS_TSK_OP_SUSPENDING);
        OsTaskOperaEnd(runTask, taskCB);
        OsSpinUnlockTaskRq(taskCB);
        OS_REPORT_ERROR(OS_ERRNO_TSK_SUSPEND_LOCKED);
        OsIntRestore(*intSave);
        return OS_ERRNO_TSK_SUSPEND_LOCKED;
    }

    taskCB->taskStatus |= OS_TSK_SUSPEND;
    return OS_OK; 
}
/*
 * 描述：suspend task
 */
OS_SEC_L2_TEXT U32 PRT_TaskSuspend(TskHandle taskPid)
{
    uintptr_t intSave;
    U32 ret;
    struct TagTskCb *taskCB = NULL;
    struct TagTskCb *runTask = NULL;

    ret = TaskParaCheck(taskPid);
    if (ret != OS_OK) {
        return ret;
    }
    taskCB = GET_TCB_HANDLE(taskPid);
    if (TSK_IS_UNUSED(taskCB)) {
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    runTask = OsGetCurrentTcb();

    ret = OsTaskSuspendPre(runTask, taskCB, &intSave);
    if (ret != OS_OK) {
        return ret;
    }
    if (taskCB == runTask) {
        OS_TSK_OP_CLR(taskCB, OS_TSK_OP_SUSPENDING);
        OsTskReadyDel(taskCB);
        OsTaskOperaEnd(runTask, taskCB);
        OsSpinUnlockTaskRq(taskCB);

        if (OS_FLG_BGD_ACTIVE & UNI_FLAG) {
            OsTskSchedule();
        }
        OsIntRestore(intSave);
        return OS_OK;
    }

    while (((volatile U32)taskCB->taskStatus) & OS_TSK_READY) {
        OsTskReadyDelSync(taskCB, &intSave);
        OsSpinLockTaskRq(taskCB);
    }

    OS_TSK_OP_CLR(taskCB, OS_TSK_OP_SUSPENDING);
    OsSpinUnlockTaskRq(taskCB);
    OsTaskOperaEnd(runTask, taskCB);
    
    if (OS_FLG_BGD_ACTIVE &UNI_FLAG) {
        OsTskSchedule();
    }
    OsIntRestore(intSave);
    return OS_OK;
}
#endif

/*
 * 描述：resume suspend task
 */
OS_SEC_L2_TEXT U32 PRT_TaskResume(TskHandle taskPid)
{
    uintptr_t intSave;
    U32 ret;
    struct TagTskCb *taskCB = NULL;

    ret = TaskParaCheck(taskPid);
    if (ret != OS_OK) {
        return ret;
    }

    struct TagOsRunQue *runQue = THIS_RUNQ();

    taskCB = GET_TCB_HANDLE(taskPid);
    if (TSK_IS_UNUSED(taskCB)) {
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    ret = OsTryLockTaskOperating(OS_TSK_OP_RESUMING, taskCB, &intSave);
    if (ret != OS_OK) {
        return ret;
    }
    /* if task is not suspended then return */
    if (!(taskCB->taskStatus & OS_TSK_SUSPEND)) {
        OS_TSK_OP_CLR(taskCB, OS_TSK_OP_RESUMING);
        OsSpinUnlockTaskRq(taskCB);
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_SUSPENDED;
    }
    
    TSK_STATUS_CLEAR(taskCB, OS_TSK_SUSPEND);
    OS_TSK_OP_CLR(taskCB, OS_TSK_OP_RESUMING);

    /* if task is not bloked then move it to ready list */
    OsMoveTaskToReady(taskCB);
    OsIntRestore(intSave);

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE bool OsTskUnlockIsNeedResched(const struct TagOsRunQue *rq)
{
    return rq->needReschedule;
}
/*
 * 描述：解锁任务调度
 * 输入：NONE
 * 输出：NONE
 * 返回：NONE
 */
OS_SEC_L0_TEXT void OsTskUnlock(void)
{
    struct TagOsRunQue *runQue = THIS_RUNQ();

    if (LIKELY(runQue->uniTaskLock == 1)) {
        runQue->uniTaskLock = 0;
        if (runQue->needReschedule == TRUE) {
            SMP_MC_SCHEDULE_TRIGGER(THIS_CORE());
        }
        return;
    }
    if (runQue->uniTaskLock > 1) {
        runQue->uniTaskLock --;
    }
    return;
}