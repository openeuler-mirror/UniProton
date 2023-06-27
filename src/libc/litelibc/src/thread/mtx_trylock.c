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
 * Description: mtx_trylock 实现
 */
#include <threads.h>
#include <pthread.h>
#include <errno.h>

int mtx_trylock(mtx_t *m)
{
    int ret = __pthread_mutex_trylock((pthread_mutex_t *)m);
    switch (ret) {
        default:
            return thrd_error;
        case 0:
            return thrd_success;
        case EBUSY:
            return thrd_busy;
    }
}
