/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-25
 * Description: shell los_task 适配头文件。
 */
#ifndef _LOS_TASK_H
#define _LOS_TASK_H

#include "prt_task_external.h"
#include "los_base.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/**
 * @ingroup los_task
 * Flag that indicates the task or task control block status. LOS_TASK_STATUS_DETACHED
 * means the task is in the auto-deleted state. In this state, the task will be deleted
 * automatically after the task is done.
 */
#define LOS_TASK_STATUS_DETACHED                0x0100U

#define LOS_TASK_PARAM_INIT_ARG(initParam, arg) \
            initParam.args[OS_TSK_PARA_0] = (uintptr_t)shellCB;

typedef struct TskInitParam TSK_INIT_PARAM_S;

typedef TskEntryFunc TSK_ENTRY_FUNC;

extern UINT32 LOS_TaskCreate(UINT32 *taskId, TSK_INIT_PARAM_S *initParam);
extern UINT32 LOS_TaskDelete(UINT32 taskId);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* _LOS_TASK_H */