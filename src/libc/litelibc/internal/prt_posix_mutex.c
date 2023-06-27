/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
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
 * Description: pthread mutex功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int OsMutexParamCheck(prt_pthread_mutex_t *mutex)
{
    int ret;
    prt_pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (memcmp(mutex, &tmp, sizeof(prt_pthread_mutex_t)) == 0) {
        ret = pthread_mutex_init(mutex, NULL);
        if (ret != OS_OK) {
            return EINVAL;
        }
    }

    return OS_OK;
}
