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

#if defined(OS_OPTION_TASK_DELETE)
/*
 * 描述：删除一个任务线程
 */
OS_SEC_L4_TEXT U32 OsTaskDelStatusCheck(struct TagTskCb *taskCb)
{
    /* If the task is running and scheduler is locked */
    /* then can not delete it */
    if ((taskCb == RUNNING_TASK) && (OS_TASK_LOCK_DATA != 0)) {
        OS_REPORT_ERROR(OS_ERRNO_TSK_DELETE_LOCKED);
        return OS_ERRNO_TSK_DELETE_LOCKED;
    }

    if (TSK_STATUS_TST(taskCb, OS_TSK_QUEUE_BUSY)) {
        return OS_ERRNO_TSK_QUEUE_DOING;
    }

    return OS_OK;
}
#endif

OS_SEC_L4_TEXT void OsTaskExit(struct TagTskCb *tsk)
{
#if defined(OS_OPTION_TASK_DELETE)
    /* 如果任务自删除失败就记录错误码，并且上报致命错误 */
    U32 ret = PRT_TaskDelete(tsk->taskPid);
    if (ret != OS_OK) {
        OS_REPORT_ERROR(ret);
        OS_REPORT_ERROR(OS_ERRNO_TSK_EXIT_WITH_RESOURCE);
    }
#else
    uintptr_t intSave = OsIntLock();

    OsTskReadyDel(tsk);
    OsTskSchedule();

    OsIntRestore(intSave);
#endif
}

#if defined(OS_OPTION_TASK_DELETE)
/*
 * 描述：删除一个任务线程
 */
OS_SEC_L4_TEXT U32 PRT_TaskDelete(TskHandle taskPid)
{
    return OsTaskDelete(taskPid);
}
#endif
