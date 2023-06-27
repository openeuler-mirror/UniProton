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
 * Description: pthread_cancel 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

static int OsPthreadCancelJoinableHandle(struct TagTskCb *tskCb)
{
    U32 ret;

    while (tskCb->cancelBuf) {
        void (*f)(void *) = tskCb->cancelBuf->__f;
        void *x = tskCb->cancelBuf->__x;
        tskCb->cancelBuf = tskCb->cancelBuf->__next;
        f(x);
    }
    tskCb->state = PTHREAD_EXITED;
    if (tskCb->joinCount == 0) {
        if (tskCb->joinableSem != 0) {
            ret = PRT_SemDelete(tskCb->joinableSem);
            if (ret != OS_OK) {
                return EINVAL;
            }
            tskCb->joinableSem = 0;
        }
        OsPthreadRunDestructor(tskCb);
    } else {
        while (tskCb->joinCount) {
            tskCb->joinCount--;
            ret = PRT_SemPost(tskCb->joinableSem);
            if (ret != OS_OK) {
                return EINVAL;
            }
        }
    }
    ret = PRT_TaskDelete(tskCb->taskPid);
    if (ret != OS_OK) {
        return EAGAIN;
    }

    return OS_OK;
}

static int OsPthreadCancelDetachedHandle(struct TagTskCb *tskCb)
{
    U32 ret;

    tskCb->state = PTHREAD_TERMINATED;
    if (tskCb->joinableSem != 0) {
        ret = PRT_SemDelete(tskCb->joinableSem);
        if (ret != OS_OK) {
            return EINVAL;
        }
        tskCb->joinableSem = 0;
    }
    while (tskCb->cancelBuf) {
        void (*f)(void *) = tskCb->cancelBuf->__f;
        void *x = tskCb->cancelBuf->__x;
        tskCb->cancelBuf = tskCb->cancelBuf->__next;
        f(x);
    }
    OsPthreadRunDestructor(tskCb);
    ret = PRT_TaskDelete(tskCb->taskPid);
    if (ret != OS_OK) {
        return EAGAIN;
    }

    return OS_OK;
}

int pthread_cancel(pthread_t thread)
{
    struct TagTskCb *tskCb;
    uintptr_t intSave;
    int ret = OS_OK;
    struct TagTskCb *curTskCb = RUNNING_TASK;

    if (CHECK_TSK_PID_OVERFLOW((TskHandle)thread)) {
        return ESRCH;
    }

    tskCb = GET_TCB_HANDLE((TskHandle)thread);
    intSave = OsIntLock();
    if ((TskHandle)thread == curTskCb->taskPid) {
        if (tskCb->cancelState == PTHREAD_CANCEL_ENABLE && tskCb->cancelType == PTHREAD_CANCEL_ASYNCHRONOUS) {
            PRT_PthreadExit(PTHREAD_CANCELED);
        }
        OsIntRestore(intSave);
        return OS_OK;
    }

    tskCb->cancelPending = 1;
    if (tskCb->cancelType == PTHREAD_CANCEL_DEFERRED) {
        OsIntRestore(intSave);
        return OS_OK;
    }
    if (tskCb->cancelState == PTHREAD_CANCEL_ENABLE) {
        if (tskCb->state == PTHREAD_CREATE_DETACHED) {
            ret = OsPthreadCancelDetachedHandle(tskCb);
        } else if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
            ret = OsPthreadCancelJoinableHandle(tskCb);
        }
    }

    OsIntRestore(intSave);

    return ret;
}