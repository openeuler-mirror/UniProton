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
 * Create: 2023-06-01
 * Description: pthread_cond_timedwait 相关接口实现
 */
#include <pthread.h>
#include <errno.h>
#include "prt_event.h"
#include "prt_posix_internal.h"

int __private_cond_wait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict m, U32 timeout)
{
    U32 intSave, eRet;
    int ret;
    ret = pthread_mutex_lock(&cond->mutex);
    if (ret != OS_OK) {
        return ret;
    }
    intSave = PRT_HwiLock();
    ListTailAdd(&RUNNING_TASK->condNode, &cond->head);
    PRT_HwiRestore(intSave);
    ret = pthread_mutex_unlock(&cond->mutex);
    if (ret != OS_OK) {
        return ret;
    }

    ret = pthread_mutex_unlock(m);
    if (ret != OS_OK) {
        return ret;
    }
    eRet = PRT_EventRead(cond->eventMask, OS_EVENT_ALL | OS_EVENT_WAIT, timeout, NULL);
    if (eRet != OS_OK) {
        eRet = (eRet == OS_ERRNO_EVENT_READ_TIMEOUT) ? ETIMEDOUT : EINVAL;
        intSave = PRT_HwiLock();
        ListDelete(&RUNNING_TASK->condNode);
        PRT_HwiRestore(intSave);
    }
    ret = pthread_mutex_lock(m);
    if (ret != OS_OK) {
        return ret;
    }
    return (int)eRet;
}

int __pthread_cond_timedwait(pthread_cond_t *restrict cond, pthread_mutex_t *restrict m, const struct timespec *restrict ts)
{
    U32 eRet;
    U32 ticks;
    if (OsCondParamCheck(cond) != OS_OK || m == NULL) {
        return EINVAL;
    }
    if (ts == NULL) {
        return __private_cond_wait(cond, m, OS_WAIT_FOREVER);
    }
    eRet = OsTimeOut2Ticks(ts, &ticks);
    if (eRet != OS_OK) {
        return (int)eRet;
    }
    return __private_cond_wait(cond, m, ticks);
}
weak_alias(__pthread_cond_timedwait, pthread_cond_timedwait);