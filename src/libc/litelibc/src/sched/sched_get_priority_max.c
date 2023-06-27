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
 * Create: 2022-11-15
 * Description: sched_get_priority_max/sched_get_priority_min 功能实现
 */
#include <sched.h>
#include "prt_posix_internal.h"

int sched_get_priority_max(int policy)
{
    if (policy < 0) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    errno = OS_OK;

    return OS_TSK_PRIORITY_HIGHEST;
}

int sched_get_priority_min(int policy)
{
    if (policy < 0) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    errno = OS_OK;

    return OS_TSK_PRIORITY_LOWEST - 1;
}
