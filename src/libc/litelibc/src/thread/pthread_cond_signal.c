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
 * Description: pthread_cond_signal 相关接口实现
 */
#include <pthread.h>
#include <errno.h>
#include "prt_event.h"
#include "prt_posix_internal.h"

int __private_cond_signal(pthread_cond_t *cond, int isOnce)
{
    int ret;
    U32 intSave, eRet;
    bool needSched = false;
    struct TagTskCb *resumedTask = NULL;
    struct TagTskCb *nextTask = NULL;
    ret = pthread_mutex_lock(&cond->mutex);
    if (ret != OS_OK) {
        return ret;
    }
    intSave = PRT_HwiLock();
    PRT_TaskLock();

    if (!ListEmpty(&cond->head)) {
        for (resumedTask = LIST_COMPONENT((&cond->head)->next, struct TagTskCb, condNode);
            &resumedTask->condNode != &cond->head;) {
                nextTask = LIST_COMPONENT(resumedTask->condNode.next, struct TagTskCb, condNode);
                needSched = true;

                eRet = PRT_EventWrite(resumedTask->taskPid, cond->eventMask);
                if ((resumedTask->taskStatus & OS_TSK_TIMEOUT) == 0) {
                    ListDelete(&resumedTask->condNode);
                }
                if (eRet != OS_OK) {
                    PRT_TaskUnlock();
                    PRT_HwiRestore(intSave);
                    (void)pthread_mutex_unlock(&cond->mutex);
                    return EINVAL;
                }
                if (isOnce == TRUE) {
                    break;
                }
                resumedTask = nextTask;
        }
    }

    PRT_TaskUnlock();
    if (needSched) {
        OsTskSchedule();
    }
    PRT_HwiRestore(intSave);
    ret = pthread_mutex_unlock(&cond->mutex);
    return ret;
}

int pthread_cond_signal(pthread_cond_t *cond)
{
    if (OsCondParamCheck(cond) != OS_OK) {
        return EINVAL;
    }
    return __private_cond_signal(cond, 1);
}