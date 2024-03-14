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
 * Description: Task Self ID Get implementation
 */
#include "prt_task_external.h"

/*
 * 描述：获取当前任务ID
 */
OS_SEC_L2_TEXT U32 PRT_TaskSelf(TskHandle *taskPid)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

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
