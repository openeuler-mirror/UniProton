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
 * Description: pthread get相关接口实现
 */
#include <errno.h>
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachState)
{
    if (attr == NULL || detachState == NULL) {
        return EINVAL;
    }

    *detachState = (int)attr->detachstate;

    return OS_OK;
}

// 不支持设置和获取guardsize
int pthread_attr_getguardsize(const pthread_attr_t *restrict a, size_t *restrict size)
{
    (void)a;
    (void)size;

	return ENOTSUP;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (attr == NULL || inheritsched == NULL) {
        return EINVAL;
    }

    *inheritsched = (int)attr->inheritsched;

    return OS_OK;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *schedparam)
{
    if (attr == NULL || schedparam == NULL) {
        return EINVAL;
    }

    schedparam->sched_priority = attr->schedparam.sched_priority;

    return OS_OK;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *schedpolicy)
{
    if (attr == NULL || schedpolicy == NULL) {
        return EINVAL;
    }

    *schedpolicy = (int)attr->schedpolicy;

    return OS_OK;
}

int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
    if (attr == NULL || scope == NULL) {
        return EINVAL;
    }

    *scope = (int)attr->scope;

    return OS_OK;
}

int pthread_attr_getstack(const pthread_attr_t *attr, void **stackAddr, size_t *stackSize)
{
    if (attr == NULL || stackAddr == NULL || stackSize == NULL) {
        return EINVAL;
    }

    *stackAddr = attr->stackaddr;
    *stackSize = attr->stacksize;

    return OS_OK;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stackSize)
{
    if (attr == NULL || stackSize == NULL || attr->stacksize_set == 0) {
        return EINVAL;
    }

    *stackSize = attr->stacksize;

    return OS_OK;
}

int pthread_barrierattr_getpshared(const pthread_barrierattr_t *restrict a, int *restrict pshared)
{
    if (a == NULL || pshared == NULL) {
        return EINVAL;
    }
	*pshared = !!a->__attr;
	return 0;
}

// 不支持获取clockid
int pthread_condattr_getclock(const pthread_condattr_t *restrict a, clockid_t *restrict clk)
{
    if (a == NULL || clk == NULL) {
        return EINVAL;
    }

    *clk = a->clock;

    return 0;
}

// 当前仅支持 PTHREAD_PROCESS_PRIVATE
int pthread_condattr_getpshared(const pthread_condattr_t *restrict a, int *restrict pshared)
{
    if (a == NULL || pshared == NULL) {
        return EINVAL;
    }

    *pshared = PTHREAD_PROCESS_PRIVATE;
    return 0;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
    if (protocol == NULL || attr == NULL) {
        return EINVAL;
    }

    *protocol = (int)attr->protocol;

    return OS_OK;
}

// 当前仅支持 PTHREAD_PROCESS_PRIVATE
int pthread_mutexattr_getpshared(const pthread_mutexattr_t *restrict a, int *restrict pshared)
{
    if (a == NULL || pshared == NULL) {
        return EINVAL;
    }
    *pshared = PTHREAD_PROCESS_PRIVATE;

	return 0;
}

// 当前仅支持 PTHREAD_MUTEX_STALLED
int pthread_mutexattr_getrobust(const pthread_mutexattr_t *restrict a, int *restrict robust)
{
    if (a == NULL || robust == NULL) {
        return EINVAL;
    }
    *robust = PTHREAD_MUTEX_STALLED;

	return 0;
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (type == NULL || attr == NULL) {
        return EINVAL;
    }

    *type = (int)attr->type;

    return OS_OK;
}

// 当前仅支持 PTHREAD_PROCESS_PRIVATE
int pthread_rwlockattr_getpshared(const pthread_rwlockattr_t *restrict a, int *restrict pshared)
{
    if (a == NULL || pshared == NULL) {
        return EINVAL;
    }
    *pshared = PTHREAD_PROCESS_PRIVATE;

	return 0;
}
