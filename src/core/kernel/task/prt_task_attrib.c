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
 * Description: Task Attribute Info implementation
 */
#include "prt_task_external.h"

/*
 * 描述：获取指定任务的状态信息
 */
OS_SEC_L4_TEXT TskStatus PRT_TaskGetStatus(TskHandle taskPid)
{
    struct TagTskCb *taskCb = NULL;

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return (TskStatus)OS_INVALID;
    }

    taskCb = GET_TCB_HANDLE(taskPid);

    return (TskStatus)taskCb->taskStatus;
}

