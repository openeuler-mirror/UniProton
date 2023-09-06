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
 * Description: pthread_join 相关接口实现
 */
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "pthread.h"
#include "prt_posix_internal.h"

static U32 OsPthreadJoinExit(struct TagTskCb *tskCb, void **status)
{
    switch (tskCb->state) {
        case PTHREAD_EXITED:
            if (status != NULL) {
                *status = tskCb->retval;
            }
            /* the last parent frees the resources */
            if (tskCb->joinCount == 0) {
                tskCb->state = PTHREAD_TERMINATED;
                if (tskCb->joinableSem != 0) {
                    PRT_SemDelete(tskCb->joinableSem);
                    tskCb->joinableSem = 0;
                }
                OsPthreadRunDestructor(tskCb);
            }
            return OS_OK;
        case PTHREAD_CREATE_JOINABLE:
        case PTHREAD_CREATE_DETACHED:
            return EINVAL;
        case PTHREAD_TERMINATED: /* fall through */
        default:
            return ESRCH;
    }
}

int PRT_PthreadTimedJoin(TskHandle thread, void **status, U32 timeout)
{
    struct TagTskCb *tskCb = RUNNING_TASK;
    uintptr_t intSave;
    U32 ret = 0;

    if (thread == tskCb->taskPid) {
        return EDEADLK;
    }
    if (CHECK_TSK_PID_OVERFLOW(thread)) {
        return ESRCH;
    }

    intSave = OsIntLock();

    tskCb = GET_TCB_HANDLE(thread);
    /* the target thread already exited */
    if (tskCb->taskStatus == OS_TSK_UNUSED) {
        ret = OsPthreadJoinExit(tskCb, status);
        OsIntRestore(intSave);
        return ret;
    }

    if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
        /* wait the target thread to finish */
        tskCb->joinCount++;
        OsIntRestore(intSave);
        ret = PRT_SemPend(tskCb->joinableSem, timeout);
        if (ret != OS_OK) {
            tskCb->joinCount--;
            return EDEADLK;
        }
        intSave = OsIntLock();
        tskCb->joinCount--;
    }

    ret = OsPthreadJoinExit(tskCb, status);
    OsIntRestore(intSave);

    return ret;
}

int __pthread_timedjoin_np(pthread_t thread, void **status, const struct timespec *at)
{
    U32 ret;
    U32 ticks;
    if (at == NULL) {
        return PRT_PthreadTimedJoin((TskHandle)thread, status, OS_WAIT_FOREVER);
    }

    if ((at->tv_sec < 0) || (at->tv_nsec < 0) || (at->tv_nsec > OS_SYS_NS_PER_SECOND)) {
        return EINVAL;
    }

    ret = OsTimeOut2Ticks(at, &ticks);
    if (ret != OS_OK) {
        return EINVAL;
    }
    return PRT_PthreadTimedJoin((TskHandle)thread, status, ticks);
}

int __pthread_tryjoin_np(pthread_t thread, void **status)
{
    return PRT_PthreadTimedJoin((TskHandle)thread, status, 0);
}


int __pthread_join(pthread_t thread, void **status)
{
    return PRT_PthreadTimedJoin((TskHandle)thread, status, OS_WAIT_FOREVER);
}
weak_alias(__pthread_tryjoin_np, pthread_tryjoin_np);
weak_alias(__pthread_timedjoin_np, pthread_timedjoin_np);
weak_alias(__pthread_join, pthread_join);