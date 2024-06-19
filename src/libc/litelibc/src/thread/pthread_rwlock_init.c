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
 * Description: pthread_rwlock_init 相关接口实现
 */
#include "prt_buildef.h"
#include "pthread.h"
#include "prt_posix_internal.h"

int pthread_rwlock_init(pthread_rwlock_t *rwl, const pthread_rwlockattr_t *attr)
{
    U32 intSave;

    (void)attr;
    if (rwl == NULL) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    // 需要初始化rwl为0，不然小概率初始化失败
    if ((rwl->rw_magic & RWLOCK_COUNT_MASK) == RWLOCK_MAGIC_NUM) {
        PRT_HwiRestore(intSave);
        return EBUSY;
    }

    rwl->rw_count = 0;
    rwl->rw_owner = NULL;
    INIT_LIST_OBJECT(&(rwl->rw_read));
    INIT_LIST_OBJECT(&(rwl->rw_write));
    rwl->rw_magic = RWLOCK_MAGIC_NUM;
#if defined(OS_OPTION_SMP)
        OsSpinLockInitInner(&rwl->rwSpinLock);
#endif
    PRT_HwiRestore(intSave);

    return OS_OK;
}