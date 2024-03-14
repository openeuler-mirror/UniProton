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
 * Description: Task Self ID Get implementation
 */
#include "prt_task_external.h"
#include "prt_smp_task_internal.h"

/*
 * 描述：获取当前任务ID
 */
OS_SEC_L2_TEXT U32 PRT_TaskSelf(TskHandle *taskPid)
{
    struct TagTskCb *tskCb = OsGetCurrentTcb();

    if (taskPid == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    /* 任务的 PID 非法 */
    if (tskCb == NULL) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    *taskPid = tskCb->taskPid;
    return OS_OK;
}

/*
 * 描述：根据核号获取该核的当前任务ID
 */
OS_SEC_L2_TEXT U32 PRT_TaskGetCurrentOnCore(U32 coreId, TskHandle *taskPid)
{
    struct TagOsRunQue *runQue = NULL;
    if (OS_COREID_CHK_INVALID(coreId)) {
        return OS_ERRNO_TSK_GET_CURRENT_COREID_INVALID;
    }

    if (taskPid == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }
    runQue = GET_RUNQ(coreId);
    if (runQue->tskCurr == NULL) {
        return OS_ERRNO_TSK_DESTCORE_NOT_RUNNING;
    }
    *taskPid = runQue->tskCurr->taskPid;
    return OS_OK;
}
/*
 * 描述：根据核号获取该核的就绪任务个数
 */
OS_SEC_L2_TEXT U32 PRT_TaskGetNrRunningOnCore(U32 coreId, U32 *nrRunning)
{
    struct TagOsRunQue *runQue = NULL;
    if (OS_COREID_CHK_INVALID(coreId)) {
        return OS_ERRNO_TSK_GET_CURRENT_COREID_INVALID;
    }

    if (nrRunning == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }
    runQue = GET_RUNQ(coreId);
    if (runQue->tskCurr == NULL) {
        return OS_ERRNO_TSK_DESTCORE_NOT_RUNNING;
    }
    *nrRunning = runQue->nrRunning;
    return OS_OK;
}