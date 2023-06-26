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
 * Description: sem_getvalue 相关接口实现
 */
#include "semaphore.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int sem_getvalue(sem_t *__restrict sem, int *__restrict val)
{
    U32 ret;
    uintptr_t intSave;
    struct TagSemCb *semCb;

    if (val == NULL || sem == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    semCb = GET_SEM(sem->semHandle);
    intSave = PRT_HwiLock();

    ret = PRT_SemGetCount(sem->semHandle, (U32 *)val);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    PRT_HwiRestore(intSave);

    return OS_OK;
}