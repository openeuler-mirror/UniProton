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
 * Description: pthread_detach 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int __pthread_detach(pthread_t thread)
{
    struct TagTskCb *tskCb;
    uintptr_t intSave;
    U32 ret = 0;

    if (CHECK_TSK_PID_OVERFLOW((TskHandle)thread)) {
        return ESRCH;
    }

    tskCb = GET_TCB_HANDLE((TskHandle)thread);
    intSave = OsIntLock();

    switch (tskCb->state) {
        case PTHREAD_CREATE_JOINABLE:
            tskCb->state = PTHREAD_CREATE_DETACHED;
            OsPthreadNotifyParents(tskCb);
            break;
        case PTHREAD_EXITED:
            pthread_join(thread, NULL);
            break;
        case PTHREAD_TERMINATED:
            ret = ESRCH;
            break;
        default:
            ret = EINVAL;
            break;
    }

    OsIntRestore(intSave);

    return ret;
}

weak_alias(__pthread_detach, pthread_detach);
weak_alias(__pthread_detach, thrd_detach);