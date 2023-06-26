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
 * Description: sem_destroy 相关接口实现
 */
#include "semaphore.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int sem_destroy(sem_t *sem)
{
    U32 ret;
    struct TagSemCb *semCb;

    if (sem == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    semCb = GET_SEM(sem->semHandle);

    ret = PRT_SemDelete(sem->semHandle);
    if (ret != OS_OK) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    (void)memset_s(semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, 0, MAX_POSIX_SEMAPHORE_NAME_LEN + 1);
    sem->semHandle = 0xffffU;
    sem->refCount = 0;

    return OS_OK;
}