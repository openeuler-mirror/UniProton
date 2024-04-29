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

/*
 * 描述：挂起任务
 */
OS_SEC_L2_TEXT U32 PRT_TaskSuspend(TskHandle taskPid)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if (taskPid == IDLE_TASK_ID) {
        return OS_ERRNO_TSK_OPERATE_IDLE;
    }

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    taskCb = GET_TCB_HANDLE(taskPid);

    intSave = OsIntLock();

    if (taskCb->taskStatus == OS_TSK_UNUSED) {
        OsIntRestore(intSave);

        return OS_ERRNO_TSK_NOT_CREATED;
    }

    /* If task is suspended then return */
    if ((OS_TSK_SUSPEND & taskCb->taskStatus) != 0) {
        OsIntRestore(intSave);

        return OS_ERRNO_TSK_ALREADY_SUSPENDED;
    }

    if (((OS_TSK_RUNNING & taskCb->taskStatus) != 0) && (g_uniTaskLock != 0)) {
        OsIntRestore(intSave);

        OS_REPORT_ERROR(OS_ERRNO_TSK_SUSPEND_LOCKED);
        return OS_ERRNO_TSK_SUSPEND_LOCKED;
    }

    /* If task is ready then remove from ready list */
    if ((OS_TSK_READY & taskCb->taskStatus) != 0) {
        OsTskReadyDel(taskCb);
    }

    taskCb->taskStatus |= OS_TSK_SUSPEND;

    if ((OS_TSK_RUNNING & taskCb->taskStatus) != 0) {
        OsTskScheduleFastPs(intSave);
    }

    OsIntRestore(intSave);
    return OS_OK;
}

/*
 * 描述解挂任务
 */
OS_SEC_L2_TEXT U32 PRT_TaskResume(TskHandle taskPid)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    taskCb = GET_TCB_HANDLE(taskPid);

    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    if (((OS_TSK_RUNNING & taskCb->taskStatus) != 0) && (g_uniTaskLock != 0)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_ACTIVE_FAILED;
    }

    /* If task is not suspended and not in interruptible delay then return */
    if (((OS_TSK_SUSPEND | OS_TSK_DELAY_INTERRUPTIBLE) & taskCb->taskStatus) == 0) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_SUSPENDED;
    }

    TSK_STATUS_CLEAR(taskCb, OS_TSK_SUSPEND);

    /* If task is not blocked then move it to ready list */
    OsMoveTaskToReady(taskCb);
    OsIntRestore(intSave);

    return OS_OK;
}
