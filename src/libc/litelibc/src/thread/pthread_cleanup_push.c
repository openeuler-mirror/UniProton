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
 * Description: _pthread_cleanup_push _pthread_cleanup_pop 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

static void OsPthreadDoCleanupPush(struct __ptcb *cb)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    cb->__next = tskCb->cancelBuf;
    tskCb->cancelBuf = cb;
}

static void OsPthreadDoCleanupPop(struct __ptcb *cb)
{
    struct TagTskCb *tskCb = RUNNING_TASK;

    tskCb->cancelBuf = cb->__next;
}

void _pthread_cleanup_push(struct __ptcb *cb, void (*f)(void *), void *x)
{
    cb->__f = f;
    cb->__x = x;
    OsPthreadDoCleanupPush(cb);
}

void _pthread_cleanup_pop(struct __ptcb *cb, int run)
{
    OsPthreadDoCleanupPop(cb);
    if (run) cb->__f(cb->__x);
}