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
 * Description: pthread_key_create 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

void (*g_pthread_keys_destor[PTHREAD_KEYS_MAX])(void *) = {0};

void OsPthreadDestructor(void *arg)
{
    (void)arg;
}

int __pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    int i;

    for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if (g_pthread_keys_destor[i] == NULL) {
            *key = (pthread_key_t)i;
            break;
        }
    }

    if (i == PTHREAD_KEYS_MAX) {
        return EAGAIN;
    }

    if (destructor != NULL) {
        g_pthread_keys_destor[i] = destructor;
    } else {
        g_pthread_keys_destor[i] = OsPthreadDestructor;
    }

    return OS_OK;
}

weak_alias(__pthread_key_create, pthread_key_create);