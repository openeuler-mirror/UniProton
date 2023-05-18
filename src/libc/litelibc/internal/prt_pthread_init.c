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
 * Description: pthread创建功能实现
 */

#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_create(pthread_t *newthread, const pthread_attr_t *attr, void *(*threadroutine)(void *), void *arg)
{
    return PRT_PthreadCreate((TskHandle *)newthread, attr, threadroutine, arg);
}

void pthread_exit(void *retval)
{
    PRT_PthreadExit(retval);
    /* if PRT_PthreadExit is ok, never comes here */
    while (1) {
    }
}