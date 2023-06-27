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
 * Description: pthread_getattr_np 相关接口实现
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <pthread.h>
#include "prt_posix_internal.h"

int pthread_getattr_np(pthread_t thread, pthread_attr_t *attr)
{
    U32 ret;
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    ret = pthread_getattr_default_np(attr);
    if (ret != OS_OK) {
        return ret;
    }

    taskCb = GET_TCB_HANDLE((TskHandle)thread);
    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    attr->schedparam.sched_priority = (int)taskCb->priority;
    attr->detachstate = (unsigned int)taskCb->state;
    attr->stackaddr = (void *)taskCb->topOfStack;
    attr->stacksize = (size_t)taskCb->stackSize;
    OsIntRestore(intSave);

    return OS_OK;
}