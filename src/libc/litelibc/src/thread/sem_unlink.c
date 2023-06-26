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
 * Description: sem_unlink 相关接口实现
 */
#include "semaphore.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"

int sem_unlink(const char *name)
{
    U32 i;
    uintptr_t intSave;
    struct TagSemCb *semCb;
    bool find = FALSE;
    U32 ret = OS_OK;

    if (name == NULL) {
        errno = EINVAL;
        return PTHREAD_OP_FAIL;
    }
    intSave = PRT_HwiLock();
    for (i = 0; i < g_maxSem; i++) {
        semCb = GET_SEM(i);
        if (strcmp((const char *)semCb->name, name) == 0) {
            find = TRUE;
            break;
        }
    }
    if (find == FALSE) {
        PRT_HwiRestore(intSave);
        errno = ENOENT;
        return PTHREAD_OP_FAIL;
    }

    if (semCb->handle.refCount == 0) {
        ret = PRT_SemDelete((SemHandle)i);
        if (ret != OS_OK) {
            PRT_HwiRestore(intSave);
            errno = EINVAL;
            return PTHREAD_OP_FAIL;
        }
    }

    (void)memset_s(semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, 0, MAX_POSIX_SEMAPHORE_NAME_LEN + 1);
    PRT_HwiRestore(intSave);

    return OS_OK;
}