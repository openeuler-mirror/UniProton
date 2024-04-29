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

#if defined(OS_OPTION_TASK_INFO)
/*
 * 描述：获取阻塞任务的信号量
 */
OS_SEC_L4_TEXT U32 PRT_TaskGetPendSem(TskHandle taskId, U16 *semId, U16 *pendState)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if ((semId == NULL) || (pendState == NULL)) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    *pendState = 0;
    *semId = (U16)OS_INVALID;
    if (CHECK_TSK_PID_OVERFLOW(taskId)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    taskCb = GET_TCB_HANDLE(taskId);

    intSave = OsIntLock();
    if (TSK_IS_UNUSED(taskCb)) {

        OsIntRestore(intSave);

        return OS_ERRNO_TSK_NOT_CREATED;
    }

    OsSpinLockTaskRq(taskCb);

    *pendState = OS_TSK_PEND & taskCb->taskStatus;

    if (*pendState == OS_TSK_PEND) {
        *semId = ((struct TagSemCb *)taskCb->taskSem)->semId;
    }

    OsSpinUnlockTaskRq(taskCb);
    OsIntRestore(intSave);

    return OS_OK;
}
#endif
