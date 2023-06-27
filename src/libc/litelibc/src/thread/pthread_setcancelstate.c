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
 * Description: pthread_setcancelstate 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int __pthread_setcancelstate(int state, int *oldstate)
{
    struct TagTskCb *tskCb = RUNNING_TASK;
    uintptr_t intSave;

    /* currently, only supports ENABLE and DISABLE */
    if (state != PTHREAD_CANCEL_ENABLE && state != PTHREAD_CANCEL_DISABLE) {
        return EINVAL;
    }

    intSave = OsIntLock();
    if (oldstate != NULL) {
        *oldstate = tskCb->cancelState;
    }
    tskCb->cancelState = state;
    if (state == PTHREAD_CANCEL_ENABLE && tskCb->cancelPending) {
        PRT_PthreadExit(PTHREAD_CANCELED);
    }
    OsIntRestore(intSave);

    return OS_OK;
}

weak_alias(__pthread_setcancelstate, pthread_setcancelstate);