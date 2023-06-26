/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-05-29
 * Description: pthread_getschedparam 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_getschedparam(pthread_t thread, int *restrict policy, struct sched_param *restrict param)
{
    U32 ret;
    TskPrior prio;
    if (param == NULL || policy == NULL) {
        return EINVAL;
    }

    *policy = PTHREAD_DEFAULT_POLICY;

    ret = PRT_TaskGetPriority((TskHandle)thread, &prio);
    if (ret != OS_OK) {
        return EINVAL;
    }

    param->sched_priority = (int)prio;

    return OS_OK;
}