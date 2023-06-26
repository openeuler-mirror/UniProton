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
 * Description: pthread_rwlock_destroy 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_rwlock_destroy(pthread_rwlock_t *rwl)
{
    U32 intSave;

    if (rwl == NULL) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if ((rwl->rw_magic & RWLOCK_COUNT_MASK) != RWLOCK_MAGIC_NUM) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    if (rwl->rw_count != 0) {
        PRT_HwiRestore(intSave);
        return EBUSY;
    }

    (void)memset_s(rwl, sizeof(pthread_rwlock_t), 0, sizeof(pthread_rwlock_t));
    PRT_HwiRestore(intSave);

    return OS_OK;
}