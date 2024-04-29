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
 * Description: pthread_mutex_lock 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int __pthread_mutex_lock(pthread_mutex_t *mutex)
{
    U32 ret;
    U32 intSave;

    if (OsMutexParamCheck(mutex) != OS_OK) {
        return EINVAL;
    }

    SEM_INIT_IRQ_LOCK(intSave);
    if (mutex->magic != MUTEX_MAGIC) {
        SEM_INIT_IRQ_UNLOCK(intSave);
        return EINVAL;
    }

    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK && OsSemBusy(mutex->mutex_sem)) {
        SEM_INIT_IRQ_UNLOCK(intSave);
        return EINVAL;
    }
    SEM_INIT_IRQ_UNLOCK(intSave);

    ret = PRT_SemPend(mutex->mutex_sem, OS_WAIT_FOREVER);
    if (ret != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

weak_alias(__pthread_mutex_lock, pthread_mutex_lock);