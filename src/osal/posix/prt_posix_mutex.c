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
 * Description: pthread mutex功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"
#include "prt_sem.h"
#include "prt_sem_external.h"

#define PTHREAD_MUTEX_TYPE_ISVALID(type) ((type) >= PTHREAD_MUTEX_NORMAL && (type) <= PTHREAD_MUTEX_DEFAULT)

#if defined(OS_POSIX_TYPE_NEWLIB)
#define MUTEX_MAGIC 0xEBU
typedef struct prt_pthread_mutex_s {
    U8 type;
    U8 magic;
    SemHandle mutex_sem;
} prt_pthread_mutex_t;
#else
#define MUTEX_MAGIC 0xEBCFDEA0U
typedef pthread_mutex_t prt_pthread_mutex_t;
#endif

bool OsSemBusy(SemHandle semHandle)
{
    struct TagSemCb *semCb = NULL;

    semCb = GET_SEM(semHandle);
    if (GET_MUTEX_TYPE(semCb->semType) != PTHREAD_MUTEX_RECURSIVE && semCb->semCount == 0 &&
        GET_SEM_TYPE(semCb->semType) == SEM_TYPE_BIN) {
        return TRUE;
    }

    return FALSE;
}

int pthread_mutexattr_init(pthread_mutexattr_t *attr)
{
    if (attr == NULL) {
        return EINVAL;
    }
    attr->type = PTHREAD_MUTEX_DEFAULT;
    attr->protocol = PTHREAD_PRIO_NONE;
    attr->is_initialized = PTHREAD_ATTR_INIT;

    return OS_OK;
}

int pthread_mutexattr_destroy(pthread_mutexattr_t *attr)
{
    U32 ret;

    if (attr == NULL) {
        return EINVAL;
    }
    ret = memset_s(attr, sizeof(pthread_mutexattr_t), 0, sizeof(pthread_mutexattr_t));
    if (ret != OS_OK) {
        OsErrRecord(ret);
        return EINVAL;
    }

    return OS_OK;
}

int PRT_PthreadMutexInit(prt_pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    U32 ret;
    U32 protocol;

    if (mutex == NULL) {
        return EINVAL;
    }

    if (attr == NULL) {
        mutex->type = PTHREAD_MUTEX_DEFAULT;
        protocol = PTHREAD_PRIO_NONE;
    } else {
        if (attr->is_initialized != PTHREAD_ATTR_INIT) {
            return EINVAL;
        }
        mutex->type = (U8)attr->type;
        protocol = (U32)attr->protocol;
    }

    switch (mutex->type) {
        case PTHREAD_MUTEX_NORMAL:
        case PTHREAD_MUTEX_ERRORCHECK:
        case PTHREAD_MUTEX_DEFAULT:
            ret = OsSemCreate(OS_SEM_FULL, SEM_TYPE_BIN | (protocol << 8), SEM_MODE_PRIOR, &mutex->mutex_sem, (U32)&mutex->mutex_sem);
            break;

        case PTHREAD_MUTEX_RECURSIVE:
            ret = OsSemCreate(OS_SEM_FULL, SEM_TYPE_BIN | (PTHREAD_MUTEX_RECURSIVE << 4) | (protocol << 8), SEM_MODE_PRIOR,
                              &mutex->mutex_sem, (U32)&mutex->mutex_sem);
            break;

        default:
            ret = EINVAL;
            break;
    }

    if (ret != OS_OK) {
        return EINVAL;
    }
    mutex->magic = MUTEX_MAGIC;

    return OS_OK;
}

int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *attr)
{
    return PRT_PthreadMutexInit((prt_pthread_mutex_t *)mutex, attr);
}

static int OsMutexParamCheck(prt_pthread_mutex_t *mutex)
{
    int ret;
#if !defined(OS_POSIX_TYPE_NEWLIB)
    prt_pthread_mutex_t tmp = PTHREAD_MUTEX_INITIALIZER;
#endif

    if (mutex == NULL) {
        return EINVAL;
    }

#if defined(OS_POSIX_TYPE_NEWLIB)
    if (*(U32 *)mutex == PTHREAD_MUTEX_INITIALIZER) {
#else
    if (memcmp(mutex, &tmp, sizeof(prt_pthread_mutex_t)) == 0) {
#endif
        ret = PRT_PthreadMutexInit(mutex, NULL);
        if (ret != OS_OK) {
            return EINVAL;
        }
    }

    return OS_OK;
}

int PRT_PthreadMutexLock(prt_pthread_mutex_t *mutex)
{
    U32 ret;
    U32 intSave;

    if (OsMutexParamCheck(mutex) != OS_OK) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if (mutex->magic != MUTEX_MAGIC) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    if (mutex->type == PTHREAD_MUTEX_ERRORCHECK && OsSemBusy(mutex->mutex_sem)) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    PRT_HwiRestore(intSave);

    ret = PRT_SemPend(mutex->mutex_sem, OS_WAIT_FOREVER);
    if (ret != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

int pthread_mutex_lock(pthread_mutex_t *mutex)
{
    return PRT_PthreadMutexLock((prt_pthread_mutex_t *)mutex);
}

int PRT_PthreadMutexUnlock(prt_pthread_mutex_t *mutex)
{
    U32 ret;
    U32 intSave;

    if (OsMutexParamCheck(mutex) != OS_OK) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if (mutex->magic != MUTEX_MAGIC) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    PRT_HwiRestore(intSave);

    ret = PRT_SemPost(mutex->mutex_sem);
    if (ret != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

int pthread_mutex_unlock(pthread_mutex_t *mutex)
{
    return PRT_PthreadMutexUnlock((prt_pthread_mutex_t *)mutex);
}

int PRT_PthreadMutexTrylock(prt_pthread_mutex_t *mutex)
{
    U32 ret;
    U32 intSave;

    if (OsMutexParamCheck(mutex) != OS_OK) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if (mutex->magic != MUTEX_MAGIC) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    if (OsSemBusy(mutex->mutex_sem)) {
        PRT_HwiRestore(intSave);
        return EBUSY;
    }
    PRT_HwiRestore(intSave);

    ret = PRT_SemPend(mutex->mutex_sem, OS_WAIT_FOREVER);
    if (ret != OS_OK) {
        return EINVAL;
    }

    return OS_OK;
}

int pthread_mutex_trylock(pthread_mutex_t *mutex)
{
    return PRT_PthreadMutexTrylock((prt_pthread_mutex_t *)mutex);
}

int PRT_PthreadMutexDestroy(prt_pthread_mutex_t *mutex)
{
    U32 ret;
    U32 intSave;

    if (OsMutexParamCheck(mutex) != OS_OK) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if (mutex->magic != MUTEX_MAGIC) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }
    PRT_HwiRestore(intSave);

    ret = PRT_SemDelete(mutex->mutex_sem);
    if (ret != OS_OK) {
        return EINVAL;
    }
    mutex->magic = 0;

    return OS_OK;
}

int pthread_mutex_destroy(pthread_mutex_t *mutex)
{
    return PRT_PthreadMutexDestroy((prt_pthread_mutex_t *)mutex);
}

int pthread_mutexattr_gettype(const pthread_mutexattr_t *attr, int *type)
{
    if (type == NULL || attr == NULL) {
        return EINVAL;
    }

    if (attr->is_initialized != PTHREAD_ATTR_INIT) {
        return EINVAL;
    }

    *type = attr->type;

    return OS_OK;
}

int pthread_mutexattr_settype(pthread_mutexattr_t *attr, int type)
{
    if (!PTHREAD_MUTEX_TYPE_ISVALID(type) || attr == NULL) {
        return EINVAL;
    }

    if (attr->is_initialized != PTHREAD_ATTR_INIT) {
        return EINVAL;
    }

    attr->type = type;

    return OS_OK;
}

int PRT_PthreadMutexTimedlock(prt_pthread_mutex_t *mutex, const struct timespec *time)
{
    U32 ret;
    U32 intSave;
    U32 ticks;

    if (time == NULL) {
        return EINVAL;
    }

    if (OsMutexParamCheck(mutex) != OS_OK) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if (mutex->magic != MUTEX_MAGIC) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    if (time->tv_sec < 0 || time->tv_nsec < 0) {
        PRT_HwiRestore(intSave);
        return EINVAL;
    }

    PRT_HwiRestore(intSave);

    ret = OsTimeOut2Ticks(time, &ticks);
    if (ret != OS_OK) {
        return (int)ret;
    }

    ret = PRT_SemPend(mutex->mutex_sem, ticks);
    if (ret != OS_OK) {
        ret = (ret == OS_ERRNO_SEM_TIMEOUT) ? ETIMEDOUT : EINVAL;
    }

    return (int)ret;
}

int pthread_mutex_timedlock(pthread_mutex_t *mutex, const struct timespec *time)
{
    return PRT_PthreadMutexTimedlock((prt_pthread_mutex_t *)mutex, time);
}

int pthread_mutexattr_setprioceiling(pthread_mutexattr_t *attr, int ceiling)
{
    (void)attr;
    (void)ceiling;

    return EINVAL;
}

int pthread_mutexattr_getprioceiling(const pthread_mutexattr_t *__restrict attr, int *ceiling)
{
    (void)attr;
    (void)ceiling;

    return EINVAL;
}

int pthread_mutexattr_setprotocol(pthread_mutexattr_t *attr, int protocol)
{
    int ret;

    if (attr == NULL) {
        return EINVAL;
    }

    if (attr->is_initialized != PTHREAD_ATTR_INIT) {
        return EINVAL;
    }

    switch (protocol) {
        case PTHREAD_PRIO_NONE:
        case PTHREAD_PRIO_INHERIT:
            attr->protocol = protocol;
            ret = OS_OK;
            break;
        case PTHREAD_PRIO_PROTECT:
            ret = ENOTSUP;
            break;
        default:
            ret = EINVAL;
            break;
	}

    return ret;
}

int pthread_mutexattr_getprotocol(const pthread_mutexattr_t *attr, int *protocol)
{
    if (protocol == NULL || attr == NULL) {
        return EINVAL;
    }

    if (attr->is_initialized != PTHREAD_ATTR_INIT) {
        return EINVAL;
    }

    *protocol = attr->protocol;

    return OS_OK;
}
