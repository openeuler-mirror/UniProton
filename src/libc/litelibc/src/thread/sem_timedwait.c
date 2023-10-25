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
 * Description: sem_timedwait 相关接口实现
 */
#include "semaphore.h"
#include "prt_posix_internal.h"

int sem_timedwait(sem_t *__restrict sem, const struct timespec *__restrict at)
{
    U32 ret;
    U32 ticks;

    if (at == NULL || sem == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    ret = OsTimeOut2Ticks(at, &ticks);
    if (ret != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    ret = PRT_SemPend(sem->semHandle, ticks);
    if (ret == OS_ERRNO_SEM_TIMEOUT) {
        errno = ETIMEDOUT;
        return PTHREAD_OP_FAIL;
    } else if (ret != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    return OS_OK;
}