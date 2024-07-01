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
 * Description: os内部pthread功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"
#if defined(OS_OPTION_SMP)
#include "smp/prt_task_internal.h"
#else
#include "amp/prt_task_internal.h"
#endif
#include "prt_err_external.h"
#include "prt_sem_external.h"

void OsPthreadNotifyParents(struct TagTskCb *tskCb)
{
    U32 count = tskCb->joinCount;

    while (count--) {
        PRT_SemPost(tskCb->joinableSem);
    }
}

void OsPthreadRunDestructor(struct TagTskCb *self)
{
    int i;
    void *val;
    void (*destructor)(void *);

    for (i = 0; i < PTHREAD_KEYS_MAX; i++) {
        if ((self->tsdUsed & (1U << (U32)i)) != 0) {
            val = self->tsd[i];
            destructor = g_pthread_keys_destor[i];
            destructor(val);
        }
    }
    self->tsdUsed = 0;
}

void PRT_PthreadExit(void *retval)
{
    U32 ret;
    uintptr_t intSave;
    struct TagTskCb *tskCb;

    intSave = OsIntLock();

    tskCb = RUNNING_TASK;
    while (tskCb->cancelBuf) {
        void (*f)(void *) = tskCb->cancelBuf->__f;
        void *x = tskCb->cancelBuf->__x;
        tskCb->cancelBuf = tskCb->cancelBuf->__next;
        f(x);
    }

    tskCb->retval = retval;
    /* thread is joinable and other threads are waitting */
    if (tskCb->state == PTHREAD_CREATE_JOINABLE && tskCb->joinCount > 0) {
        tskCb->state = PTHREAD_EXITED;
        OsPthreadNotifyParents(tskCb);
    } else {
        if (tskCb->state == PTHREAD_CREATE_JOINABLE) {
            tskCb->state = PTHREAD_EXITED;
        } else {
            tskCb->state = PTHREAD_TERMINATED;
        }
        if (tskCb->joinableSem != 0) {
            PRT_SemDelete(tskCb->joinableSem);
            tskCb->joinableSem = 0;
        }
        OsPthreadRunDestructor(tskCb);
    }

    OsIntRestore(intSave);

    ret = PRT_TaskDelete(tskCb->taskPid);
    if (ret != OS_OK) {
        OsErrRecord(ret);
    }
}