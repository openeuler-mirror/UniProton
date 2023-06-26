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
 * Description: 内部锁实现
 */
#include "prt_lock.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int PRT_Sem_Lock(U32 *lock)
{
    if (lock == NULL) {
        return PTHREAD_OP_FAIL;
    }

    if (*lock == LITE_LOCK_INITIALIZER) {
        if(OsSemCreate(OS_SEM_FULL, SEM_TYPE_BIN | (PTHREAD_PRIO_NONE << 8), SEM_MODE_PRIOR, (SemHandle *)lock, (U32)(uintptr_t)lock) != OS_OK) {
            return PTHREAD_OP_FAIL;
        }
    }

    if (PRT_SemPend((SemHandle)*lock, OS_WAIT_FOREVER) != OS_OK) {
         return PTHREAD_OP_FAIL;
    }

    return OS_OK;
}

int PRT_Sem_Unlock(const U32 *lock)
{
    if (lock == NULL || *lock == LITE_LOCK_INITIALIZER) {
        return PTHREAD_OP_FAIL;
    }

    if (PRT_SemPost((SemHandle)*lock) != OS_OK) {
        return PTHREAD_OP_FAIL;
    }

    return OS_OK;
}