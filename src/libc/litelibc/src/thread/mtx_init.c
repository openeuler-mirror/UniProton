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
 * Description: mtx_init实现
 */
#include <threads.h>
#include <pthread.h>
#include "prt_typedef.h"

int mtx_init(mtx_t *m, int type)
{
    int ret;
    int attr_type;
    attr_type = (type&mtx_recursive) ? PTHREAD_MUTEX_RECURSIVE : PTHREAD_MUTEX_NORMAL;
    pthread_mutexattr_t attr = PTHREAD_MUTEX_INITIALIZER;
    ret = pthread_mutexattr_settype(&attr, attr_type);
    if (ret != OS_OK) {
        return thrd_error;
    }
    ret = pthread_mutex_init((pthread_mutex_t *)m, &attr);
    if (ret != OS_OK) {
        return thrd_error;
    }
    return thrd_success;
}
