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
 * Description: cnd_init 实现
 */
#include <threads.h>
#include <pthread.h>
#include "prt_posix_internal.h"

int cnd_init(cnd_t *cond)
{
    int ret = pthread_cond_init((pthread_cond_t *)cond, NULL);
    if (ret != 0) {
        return thrd_error;
    }
    return thrd_success;
}