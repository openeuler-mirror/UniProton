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
 * Create: 2023-06-01
 * Description: pthread_cond_init 相关接口实现
 */
#include <pthread.h>
#include "prt_posix_internal.h"

#define COND_EVENT 0x53EC7B9DU

int pthread_cond_init(pthread_cond_t *restrict cond, const pthread_condattr_t *restrict attr)
{
    if (cond == NULL) {
        return EINVAL;
    }
    if (attr != NULL) {
        if (attr->clock != CLOCK_MONOTONIC && attr->clock != CLOCK_REALTIME) {
            return EINVAL;
        }
        cond->condAttr.clock = attr->clock;
    } else {
        cond->condAttr.clock = CLOCK_REALTIME;
    }
    cond->eventMask = COND_EVENT;
    INIT_LIST_OBJECT(&cond->head);
    if (pthread_mutex_init(&cond->mutex, NULL) != 0) {
        return EAGAIN;
    }
    return OS_OK;
}