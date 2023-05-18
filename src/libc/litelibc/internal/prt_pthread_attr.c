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
 * Description: pthread attr功能实现
 */

#include "pthread.h"
#include "sched.h"
#include "prt_posix_internal.h"

int pthread_attr_init(pthread_attr_t *attr)
{
    if (attr == NULL) {
        return ENOMEM;
    }

    attr->schedpolicy = PTHREAD_DEFAULT_POLICY;
    attr->stackaddr = NULL;
    attr->stacksize = (int)g_tskModInfo.defaultSize;
    attr->schedparam.sched_priority = PTHREAD_DEFAULT_PRIORITY;
    attr->detachstate = PTHREAD_CREATE_JOINABLE;
    attr->inheritsched = PTHREAD_EXPLICIT_SCHED;
    attr->is_initialized = PTHREAD_ATTR_INIT;

    return OS_OK;
}

int pthread_attr_destroy(pthread_attr_t *attr)
{
    if (attr != NULL && attr->is_initialized != PTHREAD_ATTR_UNINIT) {
        *attr = (pthread_attr_t){0};
        return OS_OK;
    }

    return EINVAL;
}

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachState)
{
    if (attr == NULL || detachState == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    *detachState = attr->detachstate;

    return OS_OK;
}

int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachState)
{
    if (attr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    if (detachState > PTHREAD_CREATE_JOINABLE || detachState < PTHREAD_CREATE_DETACHED) {
        return EINVAL;
    }

    attr->detachstate = detachState;

    return OS_OK;
}

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *schedpolicy)
{
    if (attr == NULL || schedpolicy == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    *schedpolicy = attr->schedpolicy;

    return OS_OK;
}

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int schedpolicy)
{
    if (attr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT || schedpolicy > PTHREAD_DEFAULT_POLICY) {
        return EINVAL;
    }

    if (schedpolicy < 0) {
        return ENOTSUP;
    }

    attr->schedpolicy = schedpolicy;

    return OS_OK;
}

int pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stackSize)
{
    if (attr == NULL || stackSize == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    *stackSize = attr->stacksize;

    return OS_OK;
}

int pthread_attr_setstacksize(pthread_attr_t *attr, size_t stackSize)
{
    U64 stackAddrLen;

    if (attr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    if (stackSize < OS_TSK_MIN_STACK_SIZE) {
        return EINVAL;
    }

    stackAddrLen = (U64)(stackSize);
    /* posix接口不实现msgq, queNum默认为0 */
    if ((uintptr_t)attr->stackaddr != 0U) {
        stackAddrLen += (uintptr_t)(attr->stackaddr);
    }
    /* 保证栈空间在4G范围内不溢出 */
    if (stackAddrLen > OS_MAX_U32) {
        return EINVAL;
    }

    attr->stacksize = (int)stackSize;

    return OS_OK;
}

int pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackAddr)
{
    if (attr == NULL || stackAddr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    *stackAddr = attr->stackaddr;

    return OS_OK;
}

int pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackAddr)
{
    U64 stackAddrLen;

    if (attr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    stackAddrLen = (U32)(attr->stacksize);
    /* posix接口不实现msgq, queNum默认为0 */
    if ((uintptr_t)stackAddr != 0U) {
        stackAddrLen += (uintptr_t)(stackAddr);
    }
    /* 保证栈空间在4G范围内不溢出 */
    if (stackAddrLen > OS_MAX_U32) {
        return EINVAL;
    }

    attr->stackaddr = stackAddr;

    return OS_OK;
}

int pthread_attr_getstack(const pthread_attr_t *attr, void **stackAddr, size_t *stackSize)
{
    if (attr == NULL || stackAddr == NULL || stackSize == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    *stackAddr = attr->stackaddr;
    *stackSize = attr->stacksize;

    return OS_OK;
}

int pthread_attr_setstack(pthread_attr_t *attr, void *stackAddr, size_t stackSize)
{
    U64 stackAddrLen;

    if (attr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    if (stackSize < (long)OS_TSK_MIN_STACK_SIZE) {
        return EINVAL;
    }

    stackAddrLen = (U64)(stackSize);
    /* posix接口不实现msgq, queNum默认为0 */
    if ((uintptr_t)stackAddr != 0U) {
        stackAddrLen += (uintptr_t)(stackAddr);
    }
    /* 保证栈空间在4G范围内不溢出 */
    if (stackAddrLen > OS_MAX_U32) {
        return EINVAL;
    }

    attr->stackaddr = stackAddr;
    attr->stacksize = (int)stackSize;

    return OS_OK;
}

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inheritsched)
{
    if (attr == NULL || inheritsched == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    *inheritsched = attr->inheritsched;

    return OS_OK;
}

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inheritsched)
{
    if (attr == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    if ((U32)inheritsched > (U32)PTHREAD_EXPLICIT_SCHED) {
        return EINVAL;
    }

    attr->inheritsched = inheritsched;

    return OS_OK;
}

int pthread_attr_getschedparam(const pthread_attr_t *attr, struct sched_param *schedparam)
{
    if (attr == NULL || schedparam == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }

    schedparam->sched_priority = attr->schedparam.sched_priority;

    return OS_OK;
}

int pthread_attr_setschedparam(pthread_attr_t *attr, const struct sched_param *schedparam)
{
    if (attr == NULL || schedparam == NULL || attr->is_initialized == PTHREAD_ATTR_UNINIT) {
        return EINVAL;
    }
    /* task 优先级范围 OS_TSK_PRIORITY_HIGHEST <= priority < OS_TSK_PRIORITY_LOWEST, 避免与idle线程优先级一致得不到调度 */
    if (schedparam->sched_priority < OS_TSK_PRIORITY_HIGHEST || schedparam->sched_priority >= OS_TSK_PRIORITY_LOWEST) {
        return ENOTSUP;
    }

    attr->schedparam.sched_priority = schedparam->sched_priority;

    return OS_OK;
}
