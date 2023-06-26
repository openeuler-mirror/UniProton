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
 * Description: sem_init 相关接口实现
 */
#include "semaphore.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int sem_init(sem_t *sem, int shared, unsigned int value)
{
    U32 ret;
    uintptr_t intSave;
    SemHandle semHandle;
    struct TagSemCb *semCb;

    (void)shared;
    if (sem == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    if (value > OS_SEM_COUNT_MAX) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    intSave = OsIntLock();

    ret = PRT_SemCreate(value, &semHandle);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    // 创建成功
    semCb = GET_SEM(semHandle);
    snprintf_s(semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, MAX_POSIX_SEMAPHORE_NAME_LEN, "defaultSem%d", semHandle);
    sem->refCount++;
    sem->semHandle = semHandle;
    OsIntRestore(intSave);

    return OS_OK;
}