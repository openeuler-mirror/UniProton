/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-12
 * Description: KAL sem wait实现。
 */
#include <errno.h>
#include "kal_ipc.h"
#include "prt_sem_external.h"
#include "prt_posix_internal.h"

int KAL_SemWait(uintptr_t semid, int value, int flag, const struct timespec *timeout)
{
    U32 ret, ticks;
    if (value != 1) {
        return ENOTSUP;
    }
    if (flag & IPC_NOWAIT) {
        ticks = OS_NO_WAIT;
    } else if (timeout == NULL) {
        ticks = OS_WAIT_FOREVER;
    } else {
        ret = OsTimeOut2Ticks(timeout, &ticks);
        if (ret != OS_OK) {
            return EINVAL;
        }
    }

    ret = PRT_SemPend((SemHandle)semid, ticks);
    switch (ret) {
        case OS_OK:
            return 0;
        case OS_ERRNO_SEM_TIMEOUT:
            return EAGAIN;
        default:
            break;
    }
    return EINVAL;
}