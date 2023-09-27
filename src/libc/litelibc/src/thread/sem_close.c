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
 * Description: sem_close 相关接口实现
 */


#include "semaphore.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int sem_close(sem_t *sem)
{
    uintptr_t intSave;
    struct TagSemCb *semCb;
    U32 ret = OS_OK;

    if (sem == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    semCb = GET_SEM(sem->semHandle);

    intSave = PRT_HwiLock();

    if (semCb->handle.refCount > 0) {
        semCb->handle.refCount--;
        /* 如果先前已调用sem_unlink，在最后一次调用sem_close时要删除信号量 */
        if ((semCb->handle.refCount == 0) && (semCb->name[0] == '\0')) {
            ret = PRT_SemDelete((SemHandle)(semCb->semId));
            if (ret != OS_OK) {
                PRT_HwiRestore(intSave);
                errno = EINVAL;
                return PTHREAD_OP_FAIL;
            }
        }
    } else {
        PRT_HwiRestore(intSave);
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }

    PRT_HwiRestore(intSave);
    return OS_OK;
}