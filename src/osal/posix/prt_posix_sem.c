/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-15
 * Description: posix sem功能实现
 */
#include "semaphore.h"
#include "prt_posix_internal.h"
#include "prt_sem_external.h"
#include "prt_sem.h"
#include "prt_sys_external.h"

int sem_init(sem_t *sem, int shared, unsigned int value)
{
    U32 ret;
    uintptr_t intSave;
    SemHandle semHandle;
    struct TagSemCb *semCb;
    char name[MAX_POSIX_SEMAPHORE_NAME_LEN + 1];
    (void)shared;

    if (sem == NULL) {
        return EINVAL;
    }
    if (value > OS_SEM_COUNT_MAX) {
        return EINVAL;
    }

    intSave = OsIntLock();

    ret = PRT_SemCreate(value, &semHandle);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return EINVAL;
    }

    // 创建成功
    semCb = GET_SEM(semHandle);
    snprintf_s(name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, MAX_POSIX_SEMAPHORE_NAME_LEN, "defaultSem%d", semHandle);
    if (strncpy_s((char *)semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, name, strlen(name) + 1) != 0) {
        OS_GOTO_SYS_ERROR1();
    }
    sem->refCount++;
    sem->semHandle = semHandle;
    OsIntRestore(intSave);

    return OS_OK;
}

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
        return NULL;
    }

    if (strlen(name) >= MAX_POSIX_SEMAPHORE_NAME_LEN) {
        return NULL;
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
            return NULL;
        }
        semCb->handle.refCount++;
        PRT_HwiRestore(intSave);
        return (sem_t *)&(semCb->handle);
    }
    if ((flags & O_CREAT) == 0) {
        PRT_HwiRestore(intSave);
        return NULL;
    }
    if (val > OS_SEM_COUNT_MAX) {
        PRT_HwiRestore(intSave);
        return NULL;
    }

    ret = PRT_SemCreate(val, &sem);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return NULL;
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

int sem_close(sem_t *sem)
{
    uintptr_t intSave;
    struct TagSemCb *semCb;

    if (sem == NULL) {
        return EINVAL;
    }

    semCb = GET_SEM(sem->semHandle);

    intSave = PRT_HwiLock();
    if (semCb->name[0] == 0) {
        PRT_HwiRestore(intSave);
        return OS_OK;
    }

    if (semCb->handle.refCount > 0) {
        semCb->handle.refCount--;
    } else {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    PRT_HwiRestore(intSave);
    return OS_OK;
}

int sem_getvalue(sem_t *__restrict sem, int *__restrict val)
{
    U32 ret;
    uintptr_t intSave;
    struct TagSemCb *semCb;

    if (val == NULL || sem == NULL) {
        return EINVAL;
    }

    semCb = GET_SEM(sem->semHandle);
    intSave = PRT_HwiLock();
    if (semCb->name[0] == 0) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    ret = PRT_SemGetCount(sem->semHandle, (U32 *)val);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    PRT_HwiRestore(intSave);

    return OS_OK;
}

int sem_wait(sem_t *sem)
{
    U32 ret;
    uintptr_t intSave;
    struct TagSemCb *semCb;

    if (sem == NULL) {
        return EINVAL;
    }
    semCb = GET_SEM(sem->semHandle);
    intSave = PRT_HwiLock();
    if (semCb->name[0] == 0) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    ret = PRT_SemPend(sem->semHandle, OS_WAIT_FOREVER);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    PRT_HwiRestore(intSave);

    return OS_OK;
}

int sem_trywait(sem_t *sem)
{
    U32 ret;
    uintptr_t intSave;
    struct TagSemCb *semCb;

    if (sem == NULL) {
        return EINVAL;
    }
    semCb = GET_SEM(sem->semHandle);
    intSave = PRT_HwiLock();
    if (semCb->name[0] == 0) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    ret = PRT_SemPend(sem->semHandle, 0);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return EAGAIN;
    }
    PRT_HwiRestore(intSave);

    return OS_OK;
}

int sem_timedwait(sem_t *__restrict sem, const struct timespec *__restrict at)
{
    U32 ret;
    U32 ticks;

    if (at == NULL) {
        return EINVAL;
    }
    if ((at->tv_sec < 0) || (at->tv_nsec < 0) || (at->tv_nsec > OS_SYS_NS_PER_SECOND)) {
        return EINVAL;
    }
    if (sem == NULL) {
        return EINVAL;
    }

    ticks = OsTimeSpec2Tick(at);
    ret = PRT_SemPend(sem->semHandle, ticks);
    if (ret == OS_ERRNO_SEM_TIMEOUT) {
        return ETIMEDOUT;
    } else if (ret != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

int sem_post(sem_t *sem)
{
    U32 ret;
    uintptr_t intSave;
    struct TagSemCb *semCb;

    if (sem == NULL) {
        return EINVAL;
    }
    semCb = GET_SEM(sem->semHandle);

    intSave = PRT_HwiLock();
    if (semCb->name[0] == 0) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    ret = PRT_SemPost(sem->semHandle);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    PRT_HwiRestore(intSave);

    return OS_OK;
}

int sem_destroy(sem_t *sem)
{
    U32 ret;
    struct TagSemCb *semCb;

    if (sem == NULL) {
        return EINVAL;
    }
    semCb = GET_SEM(sem->semHandle);

    ret = PRT_SemDelete(sem->semHandle);
    if (ret != OS_OK) {
        return EINVAL;
    }

    (void)memset_s(semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, 0, MAX_POSIX_SEMAPHORE_NAME_LEN + 1);
    sem->semHandle = 0xffffU;
    sem->refCount = 0;

    return OS_OK;
}

int sem_unlink(const char *name)
{
    U32 i;
    uintptr_t intSave;
    struct TagSemCb *semCb;
    bool find = FALSE;
    U32 ret = OS_OK;

    if (name == NULL) {
        return EINVAL;
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
        return EINVAL;
    }

    if (semCb->handle.refCount == 0) {
        ret = PRT_SemDelete((SemHandle)i);
        if (ret != OS_OK) {
            PRT_HwiRestore(intSave);
            return EINVAL;
        }
    }

    (void)memset_s(semCb->name, MAX_POSIX_SEMAPHORE_NAME_LEN + 1, 0, MAX_POSIX_SEMAPHORE_NAME_LEN + 1);
    semCb->handle.refCount = 0;
    PRT_HwiRestore(intSave);

    return OS_OK;
}
