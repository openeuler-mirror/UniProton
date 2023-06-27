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
 * Description: sem_open 相关接口实现
 */
#include "semaphore.h"
#include "prt_sem_external.h"

sem_t *sem_open(const char *name, int flags, ...)
{
    U32 i;
    U32 ret;
    U32 val;
    int mode;
    va_list arg;
    uintptr_t intSave;
    struct TagSemCb *semCb;
    bool created = FALSE;
    SemHandle sem;

    if (name == NULL) {
        errno = EINVAL;
        return SEM_FAILED;
    }

    if (strlen(name) >= MAX_POSIX_SEMAPHORE_NAME_LEN || strlen(name) == 0) {
        errno = EINVAL;
        return SEM_FAILED;
    }

    va_start(arg, flags);
    mode = (int)va_arg(arg, unsigned int);
    val = va_arg(arg, unsigned int);
    va_end(arg);
    (void)mode;
    intSave = PRT_HwiLock();
    for (i = 0; i < g_maxSem; i++) {
        semCb = GET_SEM(i);
        if (strcmp((const char *)semCb->name, name) == 0) {
            created = TRUE;
            break;
        }
    }

    if (created == TRUE) {
        if (((U32)flags & (O_EXCL | O_CREAT)) == (O_EXCL | O_CREAT)) {
            PRT_HwiRestore(intSave);
            errno = EEXIST;
            return SEM_FAILED;
        }
        semCb->handle.refCount++;
        PRT_HwiRestore(intSave);
        return (sem_t *)&(semCb->handle);
    }
    if ((flags & O_CREAT) == 0) {
        PRT_HwiRestore(intSave);
        errno = ENOENT;
        return SEM_FAILED;
    }
    if (val > OS_SEM_COUNT_MAX) {
        PRT_HwiRestore(intSave);
        errno = EINVAL;
        return SEM_FAILED;
    }

    ret = PRT_SemCreate(val, &sem);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        errno = EAGAIN;
        return SEM_FAILED;
    }

    semCb = GET_SEM(sem);
    if (strncpy_s((char *)semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, name, strlen(name) + 1) != 0) {
        OS_GOTO_SYS_ERROR1();
    }

    semCb->handle.semHandle = sem;
    semCb->handle.refCount++;
    PRT_HwiRestore(intSave);

    return (sem_t *)&(semCb->handle);
}