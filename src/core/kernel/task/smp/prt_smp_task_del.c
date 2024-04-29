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
#include "prt_sem_external.h"
#include "prt_queue_external.h"

#if defined(OS_OPTION_TASK_DELETE)
/*
 * 描述：删除一个任务，对指定任务的合法判断
 * 输入：taskPID --- 任务PID
 * 输出：NULL
 * 返回：OS_OK返回成功
 */
OS_SEC_L4_TEXT U32 OsTaskDeleteCheckAndBegin(TskHandle taskPID, struct TagTskCb *runTask,
                                             uintptr_t *intSave, bool *isNeedSche)
{
    U32 ret;
    struct TagTskCb *taskCB;

    if (CHECK_TSK_PID_OVERFLOW(taskPID)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    if (TSK_GET_INDEX(taskPID) >= g_tskMaxNum) {
        return OS_ERRNO_TSK_OPERATE_IDLE;
    }

    taskCB = GET_TCB_HANDLE(taskPID);

    if (TSK_IS_UNUSED(taskCB)) {
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    ret = OsTaskOperaBegin(runTask, taskCB);
    if (ret != OS_OK) {
        return ret;
    }

    ret = OsTryLockTaskOperating(OS_TSK_OP_DELETING, taskCB, intSave);
    if (ret != OS_OK) {
        OsTaskOperaEnd(runTask, taskCB);
        return ret;
    }

    taskCB->taskStatus |= OS_TSK_DELETING;

    if (taskCB == runTask) {
        OsTskReadyDel(taskCB);
    } else if (taskCB->taskStatus & OS_TSK_READY) {
        do {
            OsTskReadyDelSync(taskCB, intSave);
            OsSpinLockTaskRq(taskCB);
        } while (((volatile U32)(taskCB->taskStatus) & OS_TSK_READY) != 0);
    } else {
        *isNeedSche = FALSE;
    }
    return OS_OK;
}
OS_SEC_ALW_INLINE INLINE void OsTaskDeleteFailed(struct TagTskCb *taskCB, struct TagTskCb *runTask,
                                                bool isNeedEnq, uintptr_t intSave)
{
    taskCB->taskStatus &= ~OS_TSK_DELETING;
    taskCB->taskOperating = OS_TSK_OP_FREE;
    if ((isNeedEnq) && (!(taskCB->taskStatus & OS_TSK_BLOCK)) && (!TSK_IS_UNUSED(taskCB))) {
        OsTskReadyAdd(taskCB);
    }
    OsSpinUnlockTaskRq(taskCB);

    OsTaskOperaEnd(runTask, taskCB);

    OsTskSchedule();
    OsIntRestore(intSave);
    return;
}

OS_SEC_L4_TEXT void OsTaskDeleteResFree(TskHandle taskPID, struct TagTskCb *taskCB)
{
    OS_MHOOK_ACTIVATE_PARA1(OS_HOOK_TSK_DELETE, taskPID);

    /* 任务阻塞 */
    if (TSK_STATUS_TST(taskCB, OS_TSK_PEND)) {
        struct TagSemCb *pendSem = taskCB->taskSem;
        SEM_CB_LOCK(pendSem);
        OsSemPrioLock();
        ListDelete(&taskCB->pendList);
        OsSemPrioUnLock();
        SEM_CB_UNLOCK(pendSem);
    }

    if (TSK_STATUS_TST(taskCB, OS_TSK_DELAY | OS_TSK_TIMEOUT)) {
        OS_TSK_DELAY_LOCKED_DETACH(taskCB);
    }
    return;
}
OS_SEC_L4_TEXT void OsTaskDeleteStatusInit(TskHandle taskPID, struct TagTskCb *taskCB)
{
    OsSpinLockTaskRq(taskCB);
    taskCB->taskStatus = OS_TSK_UNUSED;
    taskCB->taskStatus &= ~OS_TSK_DELETING;
    OsSpinUnlockTaskRq(taskCB);

    return;
}

/*
 * 描述：删除一个任务线程
 * 输入：taskPID --- 任务PID
 * 输出：NULL
 * 返回：OS_OK返回成功
 */
OS_SEC_L4_TEXT U32 OsTaskDelete(TskHandle taskPID)
{
    uintptr_t intSave;
    struct TagTskCb *taskCB = NULL;
    struct TagTskCb *runTask = NULL;

    bool isNeedEnq = TRUE;
    U32 ret;

    intSave = OsIntLock();
    runTask = RUNNING_TASK;
    OsIntRestore(intSave);

    ret = OsTaskDeleteCheckAndBegin(taskPID, runTask, &intSave, &isNeedEnq);
    if (ret != OS_OK) {
        return ret;
    }
    taskCB = GET_TCB_HANDLE(taskPID);

    ret = OsTaskDelStatusCheck(taskCB);
    if (ret != OS_OK) {
        OsTaskDeleteFailed(taskCB, runTask, isNeedEnq, intSave);
        return ret;
    }

    OsSpinUnlockTaskRq(taskCB);
    OsTaskDeleteResFree(taskPID, taskCB);
    OsTaskOperaEnd(runTask, taskCB);

    OsTaskDeleteStatusInit(taskPID, taskCB);
    if(taskCB == runTask) {
        runTask->taskStatus &= (~(OS_TSK_RUNNING | OS_TSK_READY));

        ListTailAdd(&taskCB->pendList, &g_tskPercpuRecycleList[THIS_CORE()]);
        
        OsHwiMcTrigger(OS_TYPE_TRIGGER_TO_SELF, 0, OS_SMP_MC_CORE_IPC_SGI);

        OsTskSchedule();

        OsIntRestore(intSave);
        return OS_OK;
    }
    
    TSK_CREATE_DEL_LOCK();
    ListTailAdd(&taskCB->pendList, &g_tskCbFreeList);
    TSK_CREATE_DEL_UNLOCK();
    OsTskResRecycle(taskCB);

    OsIntRestore(intSave);
    /* if deleting current task this is unreachable */
    return OS_OK;
}
#endif