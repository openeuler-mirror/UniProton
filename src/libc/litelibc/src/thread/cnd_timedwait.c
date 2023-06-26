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
 * Description: cnd_timedwait 实现
 */
#include <threads.h>
#include <pthread.h>
#include <errno.h>

int cnd_timedwait(cnd_t *restrict c, mtx_t *restrict m, const struct timespec *restrict ts)
{
    int ret = __pthread_cond_timedwait((pthread_cond_t *)c, (pthread_mutex_t *)m, ts);
    switch (ret) {
        /* May also return EINVAL or EPERM. */
        default:
            return thrd_error;
        case 0:
            return thrd_success;
        case ETIMEDOUT: 
            return thrd_timedout;
    }
}