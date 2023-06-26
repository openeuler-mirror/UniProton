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
 * Description: pthread_setattr_default_np 相关接口实现
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include "prt_posix_internal.h"

static pthread_rwlock_t lock = PTHREAD_RWLOCK_INITIALIZER;

static pthread_attr_t g_pthreadattr = {
    .schedpolicy = PTHREAD_DEFAULT_POLICY,
    .stackaddr = NULL,
    .stackaddr_set = 0,
    .stacksize = 0,
    .stacksize_set = 0,
    .schedparam = {
        .sched_priority = PTHREAD_DEFAULT_PRIORITY,
    },
    .detachstate = PTHREAD_CREATE_JOINABLE,
    .inheritsched = PTHREAD_EXPLICIT_SCHED,
    .scope = PTHREAD_SCOPE_SYSTEM
};

int pthread_setattr_default_np(const pthread_attr_t *attr)
{
    int ret;
    if (attr == NULL || attr->stackaddr != 0) {
        return EINVAL;
    }
    pthread_rwlock_wrlock(&lock);
    ret = pthread_attr_setschedpolicy(&g_pthreadattr, attr->schedpolicy);
    if (ret != 0) {
        goto ERR;
    }
    ret = pthread_attr_setstacksize(&g_pthreadattr, attr->stacksize);
    if (ret != 0) {
        goto ERR;
    }
    ret = pthread_attr_setschedparam(&g_pthreadattr, &attr->schedparam);
    if (ret != 0) {
        goto ERR;
    }
    ret = pthread_attr_setdetachstate(&g_pthreadattr, attr->detachstate);
    if (ret != 0) {
        goto ERR;
    }
    ret = pthread_attr_setinheritsched(&g_pthreadattr, attr->inheritsched);
    if (ret != 0) {
        goto ERR;
    }
    ret = pthread_attr_setscope(&g_pthreadattr, attr->scope);
    if (ret != 0) {
        goto ERR;
    }
ERR:
    pthread_rwlock_unlock(&lock);
	return ret;
}

int pthread_getattr_default_np(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }
    pthread_rwlock_rdlock(&lock);
    attr->schedpolicy = g_pthreadattr.schedpolicy;
    attr->stackaddr = g_pthreadattr.stackaddr;
    attr->stackaddr_set = g_pthreadattr.stackaddr_set;
    if (g_pthreadattr.stacksize == 0) {
        g_pthreadattr.stacksize = (size_t)g_tskModInfo.defaultSize;
        g_pthreadattr.stacksize_set = 1;
    }
    attr->stacksize = g_pthreadattr.stacksize;
    attr->stacksize_set = g_pthreadattr.stacksize_set;
    attr->schedparam.sched_priority = g_pthreadattr.schedparam.sched_priority;
    attr->detachstate = g_pthreadattr.detachstate;
    attr->inheritsched = g_pthreadattr.inheritsched;
    attr->scope = g_pthreadattr.scope;
    pthread_rwlock_unlock(&lock);
    return OS_OK;
}