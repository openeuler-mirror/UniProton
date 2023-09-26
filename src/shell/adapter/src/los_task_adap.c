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
 * Description: shell los task 适配实现。
 */
#include "los_task.h"

UINT32 LOS_TaskCreate(UINT32 *taskId, TSK_INIT_PARAM_S *initParam)
{
    U32 ret;
    ret = PRT_TaskCreate(taskId, initParam);
    if (ret != OS_OK) {
        return ret;
    }
    ret = PRT_TaskResume(*taskId);
    if (ret != OS_OK) {
        (void)PRT_TaskDelete(*taskId);
    }
    return ret;
}

UINT32 LOS_TaskDelete(UINT32 taskId)
{
    return PRT_TaskDelete(taskId);
}