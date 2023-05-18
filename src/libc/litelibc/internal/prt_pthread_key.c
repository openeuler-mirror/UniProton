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
 * Description: pthread key功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_key_create(pthread_key_t *key, void (*destructor)(void *))
{
    return PRT_PthreadKeyCreate(key, destructor);
}

int pthread_key_delete(pthread_key_t key)
{
    return PRT_PthreadKeyDelete(key);
}

int pthread_setspecific(pthread_key_t key, const void *value)
{
    return PRT_PthreadSetSpecific(key, value);
}

void *pthread_getspecific(pthread_key_t key)
{
    return PRT_PthreadGetSpecific(key);
}