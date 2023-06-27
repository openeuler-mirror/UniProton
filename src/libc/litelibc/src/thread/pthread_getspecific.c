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
 * Description: pthread_getspecific 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

void *__pthread_getspecific(pthread_key_t key)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    if ((U32)key >= PTHREAD_KEYS_MAX) {
        return NULL;
    }
    if ((tskCb->tsdUsed & (1U << (U32)key)) == 0) {
        return NULL;
    }

    return tskCb->tsd[key];
}

weak_alias(__pthread_getspecific, pthread_getspecific);
weak_alias(__pthread_getspecific, tss_get);