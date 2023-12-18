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
 * Description: pthread_rwlock_unlock 相关接口实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"

/*
 * 读写锁解锁
 */
int __pthread_rwlock_unlock(pthread_rwlock_t *rwl)
{
    U32 ret;
    U32 intSave;
    bool needSched = FALSE;

    intSave = PRT_HwiLock();
    ret = OsRwLockUnlock(rwl, &needSched);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return (int)ret;
    }

    if (needSched == TRUE) {
        OsTskSchedule();
    }
    PRT_HwiRestore(intSave);

    return (int)ret;
}
weak_alias(__pthread_rwlock_unlock, pthread_rwlock_unlock);