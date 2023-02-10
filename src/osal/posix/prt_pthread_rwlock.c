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
 * Description: pthread rwlock功能实现
 */
#include "pthread.h"
#include "prt_posix_internal.h"
#include "../../core/ipc/rwlock/prt_rwlock_internal.h"
#include "prt_mem.h"

#if defined(OS_POSIX_TYPE_NEWLIB)
prt_pthread_rwlock_t g_rwlist_head = {0};

OS_SEC_ALW_INLINE INLINE U32 OsRwlockGetMagic(pthread_rwlock_t *rwlock)
{
    return ((*(U32 *)rwlock & 0xffff0000U) >> 16U);
}

OS_SEC_ALW_INLINE INLINE void OsRwlockSetMagic(pthread_rwlock_t *rwlock, U32 val)
{
    *(U32 *)rwlock = (*(U32 *)rwlock & 0xffffU) | (val << 16U);
}

OS_SEC_ALW_INLINE INLINE U32 OsRwlockGetIndex(pthread_rwlock_t *rwlock)
{
    return (*(U32 *)rwlock & 0xffffU);
}

OS_SEC_ALW_INLINE INLINE void OsRwlockSetIndex(pthread_rwlock_t *rwlock, U32 val)
{
    *(U32 *)rwlock = (*(U32 *)rwlock & 0xffff0000U) | (val & 0xffffU);
}
#endif

static prt_pthread_rwlock_t *OsRwlock2InnerStruct(pthread_rwlock_t *rwl)
{
#if defined(OS_POSIX_TYPE_NEWLIB)
    prt_pthread_rwlock_t *rwlock = NULL;
    prt_pthread_rwlock_t *ptr = &g_rwlist_head;

    if (rwl == NULL) {
        return NULL;
    }
    if (OsRwlockGetMagic(rwl) != RWLOCK_MAGIC_NUM) {
        return NULL;
    }
    while (ptr != NULL) {
        if (OsRwlockGetIndex(rwl) == ptr->index) {
            rwlock = ptr;
            break;
        }
        ptr = ptr->next;
    }

    return rwlock;
#else
    return (prt_pthread_rwlock_t *)rwl;
#endif
}

/*
 * 初始化读写锁
 */
int PRT_PthreadRwlockInit(prt_pthread_rwlock_t *rwl, const pthread_rwlockattr_t *attr)
{
    U32 intSave;

    (void)attr;
    if (rwl == NULL) {
        return EINVAL;
    }

    intSave = PRT_HwiLock();
    if ((rwl->rw_magic & RWLOCK_COUNT_MASK) == RWLOCK_MAGIC_NUM) {
        PRT_HwiRestore(intSave);
        return EBUSY;
    }

    rwl->rw_count = 0;
    rwl->rw_owner = NULL;
    INIT_LIST_OBJECT(&(rwl->rw_read));
    INIT_LIST_OBJECT(&(rwl->rw_write));
    rwl->rw_magic = RWLOCK_MAGIC_NUM;
    PRT_HwiRestore(intSave);

    return OS_OK;
}

int pthread_rwlock_init(pthread_rwlock_t *rwl, const pthread_rwlockattr_t *attr)
{
#if defined(OS_POSIX_TYPE_NEWLIB)
    prt_pthread_rwlock_t *rwlock;
    prt_pthread_rwlock_t *ptr = &g_rwlist_head;
    U16 index = 0;

    if (rwl == NULL) {
        return EINVAL;
    }
    if (OsRwlockGetMagic(rwl) == RWLOCK_MAGIC_NUM) {
        return EBUSY;
    }
    rwlock = PRT_MemAlloc(0, 0, sizeof(prt_pthread_rwlock_t));
    if (rwlock == NULL) {
        return ENOMEM;
    }
    while (ptr->next != NULL) {
        if ((index + 1) != ptr->index) {
            break;
        }
        index = ptr->index;
        ptr = ptr->next;
    }
    rwlock->index = index + 1;
    OsRwlockSetMagic(rwl, RWLOCK_MAGIC_NUM);
    OsRwlockSetIndex(rwl, index + 1);
    rwlock->next = ptr->next;
    ptr->next = rwlock;

    return PRT_PthreadRwlockInit(rwlock, attr);
#else
    return PRT_PthreadRwlockInit((prt_pthread_rwlock_t *)rwl, attr);
#endif
}

/*
 * 销毁读写锁
 */
int PRT_PthreadRwlockDestroy(prt_pthread_rwlock_t *rwl)
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

    (void)memset_s(rwl, sizeof(prt_pthread_rwlock_t), 0, sizeof(prt_pthread_rwlock_t));
    PRT_HwiRestore(intSave);

    return OS_OK;
}

int pthread_rwlock_destroy(pthread_rwlock_t *rwl)
{
#if defined(OS_POSIX_TYPE_NEWLIB)
    prt_pthread_rwlock_t *rwlock = NULL;
    prt_pthread_rwlock_t *ptr = &g_rwlist_head;
    U32 ret;

    if (rwl == NULL) {
        return EINVAL;
    }
    if (OsRwlockGetMagic(rwl) != RWLOCK_MAGIC_NUM) {
        return EINVAL;
    }
    while (ptr != NULL) {
        if (OsRwlockGetIndex(rwl) == ptr->index) {
            rwlock = ptr;
            break;
        }
        ptr = ptr->next;
    }

    ret = PRT_PthreadRwlockDestroy(rwlock);
    if (ret != OS_OK) {
        return (int)ret;
    }
    OsRwlockSetIndex(rwl, 0);
    OsRwlockSetMagic(rwl, 0);
    ptr->next = rwlock->next;
    rwlock->next = NULL;
    
    ret = PRT_MemFree(0, rwlock);
    if (ret != OS_OK) {
        OsErrRecord(ret);
    }
    return OS_OK;
#else
    return PRT_PthreadRwlockDestroy((prt_pthread_rwlock_t *)rwl);
#endif
}

/*
 * 阻塞获取读锁
 */
int pthread_rwlock_rdlock(pthread_rwlock_t *rwl)
{
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    return (int)OsRwLockRdPend(rwlock, OS_WAIT_FOREVER, RWLOCK_RD);
}

/*
 * 非阻塞获取读锁
 */
int pthread_rwlock_tryrdlock(pthread_rwlock_t *rwl)
{
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    return (int)OsRwLockRdPend(rwlock, 0, RWLOCK_TRYRD);
}

/*
 * 带超时获取读锁
 */
int pthread_rwlock_timedrdlock(pthread_rwlock_t *rwl, const struct timespec *time)
{
    U32 ret;
    U32 ticks;
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    if (time == NULL) {
        return EINVAL;
    }

    ret = OsTimeOut2Ticks(time, &ticks);
    if (ret != OS_OK) {
        return (int)ret;
    }

    return (int)OsRwLockRdPend(rwlock, ticks, RWLOCK_TIMERD);
}

/*
 * 阻塞获取写锁
 */
int pthread_rwlock_wrlock(pthread_rwlock_t *rwl)
{
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    return (int)OsRwLockWrPend(rwlock, OS_WAIT_FOREVER, RWLOCK_WR);
}

/*
 * 非阻塞获取写锁
 */
int pthread_rwlock_trywrlock(pthread_rwlock_t *rwl)
{
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    return (int)OsRwLockWrPend(rwlock, 0, RWLOCK_TRYWR);
}

/*
 * 带超时获取写锁
 */
int pthread_rwlock_timedwrlock(pthread_rwlock_t *rwl, const struct timespec *time)
{
    U32 ret;
    U32 ticks;
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    if (time == NULL) {
        return EINVAL;
    }

    ret = OsTimeOut2Ticks(time, &ticks);
    if (ret != OS_OK) {
        return (int)ret;
    }

    return (int)OsRwLockWrPend(rwlock, ticks, RWLOCK_TIMEWR);
}

/*
 * 读写锁解锁
 */
int pthread_rwlock_unlock(pthread_rwlock_t *rwl)
{
    U32 ret;
    U32 intSave;
    bool needSched = FALSE;
    prt_pthread_rwlock_t *rwlock = OsRwlock2InnerStruct(rwl);

    intSave = PRT_HwiLock();
    ret = OsRwLockUnlock(rwlock, &needSched);
    if (ret != OS_OK) {
        PRT_HwiRestore(intSave);
        return (int)ret;
    }

    PRT_HwiRestore(intSave);
    if (needSched == TRUE) {
        OsTskSchedule();
    }

    return (int)ret;
}

/*
 * 读写锁属性销毁
 */
int pthread_rwlockattr_destroy(pthread_rwlockattr_t *attr)
{
    (void)attr;

    return OS_OK;
}

/*
 * 读写锁属性初始化
 */
int pthread_rwlockattr_init(pthread_rwlockattr_t *attr)
{
    (void)attr;

    return OS_OK;
}

