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

#if defined(OS_OPTION_TASK_DELETE)

OS_SEC_L4_TEXT void OsTaskDeleteResInit(struct TagTskCb *taskCb)
{

    if (((OS_TSK_PEND | OS_TSK_QUEUE_PEND) & taskCb->taskStatus) != 0) {
        ListDelete(&taskCb->pendList);
    }

    if (((OS_TSK_DELAY | OS_TSK_TIMEOUT) & taskCb->taskStatus) != 0) {
        ListDelete(&taskCb->timerList);
    }

    if ((OS_TSK_READY & taskCb->taskStatus) != 0) {
        OsTskReadyDel(taskCb);
    }

#if defined(OS_OPTION_LINUX)
    if ((OS_TSK_WAITQUEUE_PEND & taskCb->taskStatus) != 0) {
        ListDelete(&taskCb->waitList);
    }
#endif

    taskCb->taskStatus &= (~(OS_TSK_SUSPEND));
    taskCb->taskStatus |= OS_TSK_UNUSED;

    return;
}

/*
 * 描述：删除一个任务线程
 */
OS_SEC_L4_TEXT U32 OsTaskDelete(TskHandle taskPid)
{
    U32 ret;
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;
#if defined(OS_OPTION_LINUX)
    struct task_struct *kthreadTsk = NULL;
#endif

    if (taskPid == IDLE_TASK_ID) {
        return OS_ERRNO_TSK_OPERATE_IDLE;
    }

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    intSave = OsIntLock();

    taskCb = GET_TCB_HANDLE(taskPid);
#if defined(OS_OPTION_LINUX)
    kthreadTsk = taskCb->kthreadTsk;
    taskCb->kthreadTsk = NULL;
#endif
    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    ret = OsTaskDelStatusCheck(taskCb);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    OsTaskDeleteResInit(taskCb);

    OsTskHighestSet();

    if ((OS_TSK_RUNNING & taskCb->taskStatus) != 0) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        ListTailAdd(&taskCb->pendList, &g_tskRecyleList);

        RUNNING_TASK = OS_PST_ZOMBIE_TASK;
        RUNNING_TASK->taskPid = taskPid;
        RUNNING_TASK->taskStatus = taskCb->taskStatus;

        taskCb->taskStatus = OS_TSK_UNUSED;

        if (OS_INT_INACTIVE) {
            OsTaskTrap();
            goto release_and_exit;
        }
    } else {
        taskCb->taskStatus = OS_TSK_UNUSED;
        ListAdd(&taskCb->pendList, &g_tskCbFreeList);
        OsTskResRecycle(taskCb);
    }

release_and_exit:
    OsIntRestore(intSave);
#if defined(OS_OPTION_LINUX)
    if (kthreadTsk != NULL) {
        if (kthreadTsk->name != NULL) {
            OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)kthreadTsk->name));
        }
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)kthreadTsk));
    }
#endif
    /* if deleteing current task this is unreachable. */
    return OS_OK;
}

#endif

