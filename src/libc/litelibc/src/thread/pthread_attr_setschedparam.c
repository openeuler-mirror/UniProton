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
 * Description: pthread_attr_setschedparam 接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *schedparam)
{
    if (attr == NULL || schedparam == NULL) {
        return EINVAL;
    }
    /* task 优先级范围 OS_TSK_PRIORITY_HIGHEST <= priority < OS_TSK_PRIORITY_LOWEST, 避免与idle线程优先级一致得不到调度 */
    if (schedparam->sched_priority < OS_TSK_PRIORITY_HIGHEST || schedparam->sched_priority >= OS_TSK_PRIORITY_LOWEST) {
        return ENOTSUP;
    }

    attr->schedparam.sched_priority = schedparam->sched_priority;

    return OS_OK;
}