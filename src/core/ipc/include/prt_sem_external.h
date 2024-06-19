/*
 * Copyright (c) 2009-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2009-12-22
 * Description: 信号量模块内部头文件
 */
#ifndef PRT_SEM_EXTERNAL_H
#define PRT_SEM_EXTERNAL_H

#include "prt_sem.h"
#include "prt_task_external.h"
#if defined(OS_OPTION_SMP)
#include "prt_raw_spinlock_external.h"
#endif
#if defined(OS_OPTION_POSIX)
#include "bits/semaphore_types.h"
#endif

#define OS_SEM_UNUSED 0
#define OS_SEM_USED   1

#define SEM_PROTOCOL_PRIO_INHERIT 1
#define SEM_TYPE_BIT_WIDTH        0x4U
#define SEM_PROTOCOL_BIT_WIDTH    0x8U

#define OS_SEM_WITH_LOCK_FLAG    1
#define OS_SEM_WITHOUT_LOCK_FLAG 0

#define MAX_POSIX_SEMAPHORE_NAME_LEN    31

#define GET_SEM_LIST(ptr) LIST_COMPONENT(ptr, struct TagSemCb, semList)
#define GET_SEM(semid) (((struct TagSemCb *)g_allSem) + (semid))
#define GET_SEM_TSK(semid) (((SEM_TSK_S *)g_semTsk) + (semid))
#define GET_TSK_SEM(tskid) (((TSK_SEM_S *)g_tskSem) + (tskid))
#define GET_SEM_TYPE(semType) (U32)((semType) & ((1U << SEM_TYPE_BIT_WIDTH) - 1))
#define GET_MUTEX_TYPE(semType) (U32)(((semType) >> SEM_TYPE_BIT_WIDTH) & ((1U << SEM_TYPE_BIT_WIDTH) - 1))
#define GET_SEM_PROTOCOL(semType) (U32)((semType) >> SEM_PROTOCOL_BIT_WIDTH)
#if defined(OS_OPTION_SMP)
extern volatile uintptr_t g_semPrioLock;
#define SEM_CB_LOCK(sem) OS_MCMUTEX_LOCK(0, &(sem)->semLock)
#define SEM_CB_UNLOCK(sem) OS_MCMUTEX_UNLOCK(0, &(sem)->semLock)
#define RW_CB_LOCK(rwSpin) OS_MCMUTEX_LOCK(0, rwSpin)
#define RW_CB_UNLOCK(rwSpin) OS_MCMUTEX_UNLOCK(0, rwSpin)
#define SEM_CB_IRQ_LOCK(sem, intSave)           \
    do {                                        \
        (intSave) = OsIntLock();                \
        SEM_CB_LOCK(sem);                       \
    } while (0)
#define SEM_CB_IRQ_UNLOCK(sem, intSave)         \
    do {                                        \
        SEM_CB_UNLOCK(sem);                     \
        OsIntRestore(intSave);                  \
    } while (0)
#define SEM_INIT_IRQ_LOCK(intSave) OS_MCMUTEX_IRQ_LOCK(0, &g_mcInitGuard, (intSave))
#define SEM_INIT_IRQ_UNLOCK(intSave) OS_MCMUTEX_IRQ_UNLOCK(0, &g_mcInitGuard, (intSave))
#define SEM_INIT_LOCK() OS_MCMUTEX_LOCK(0, &g_mcInitGuard)
#define SEM_INIT_UNLOCK() OS_MCMUTEX_UNLOCK(0, &g_mcInitGuard)
#else
#define SEM_CB_LOCK(sem) (void)(sem)
#define SEM_CB_UNLOCK(sem) (void)(sem)
#define RW_CB_LOCK(rwSpin) (void)(rwSpin)
#define RW_CB_UNLOCK(rwSpin) (void)(rwSpin)
#define SEM_CB_IRQ_LOCK(sem, intSave)           \
    do {                                        \
        (void)(sem);                            \
        ((intSave) = OsIntLock());              \
    } while (0)
#define SEM_CB_IRQ_UNLOCK(sem, intSave)         \
    do {                                        \
        (void)(sem);                            \
        (OsIntRestore(intSave));                  \
    } while (0)
#define SEM_INIT_IRQ_LOCK(intSave) ((intSave) = OsIntLock())
#define SEM_INIT_IRQ_UNLOCK(intSave) (OsIntRestore(intSave))
#define SEM_INIT_LOCK() (void)NULL
#define SEM_INIT_UNLOCK() (void)NULL
#endif

struct TagSemCb {
#if defined(OS_OPTION_SMP)
    volatile uintptr_t semLock;
#endif
    /* 是否使用 OS_SEM_UNUSED/OS_SEM_USED */
    U16 semStat;
    /* 核内信号量索引号 */
    U16 semId;
#if defined(OS_OPTION_SEM_RECUR_PV)
    /* 二进制互斥信号量递归P计数，计数型信号量和二进制同步模式信号量无效 */
    U32 recurCount;
#endif
    /* 当该信号量已用时，其信号量计数 */
    U32 semCount;
    /* 挂接阻塞于该信号量的任务 */
    struct TagListObject semList;
    /* 挂接任务持有的互斥信号量，计数型信号量信号量无效 */
    struct TagListObject semBList;

    /* Pend到该信号量的线程ID */
    U32 semOwner;
    /* 信号量唤醒阻塞任务的方式 */
    enum SemMode semMode;
    /* 信号量，计数型或二进制 */
    U32 semType;
#if defined(OS_OPTION_POSIX)
    /* 信号量名称 */
    char name[MAX_POSIX_SEMAPHORE_NAME_LEN + 1]; // + \0
    /* sem_open 句柄 */
    sem_t handle;
#endif
};

/* 模块间全局变量声明 */
extern U16 g_maxSem;

/* 指向核内信号量控制块 */
extern struct TagSemCb *g_allSem;

#if defined(OS_OPTION_SMP)
// 需要保证锁顺序避免死锁，在semCb锁内，在task rq锁外
OS_SEC_ALW_INLINE INLINE void OsSemPrioLock(void)
{
    OS_MCMUTEX_LOCK(0, &g_semPrioLock);
}

OS_SEC_ALW_INLINE INLINE void OsSemPrioUnLock(void)
{
    OS_MCMUTEX_UNLOCK(0, &g_semPrioLock);
}

// 需要保证锁顺序避免死锁，在semCb锁内，在task rq锁外
OS_SEC_ALW_INLINE INLINE void OsSemIfPrioLock(struct TagSemCb *sem)
{
   if (sem->semMode == SEM_MODE_PRIOR) {
        OsSemPrioLock();
   }
}

OS_SEC_ALW_INLINE INLINE void OsSemIfPrioUnLock(struct TagSemCb *sem)
{
   if (sem->semMode == SEM_MODE_PRIOR) {
        OsSemPrioUnLock();
   }
}
#else
OS_SEC_ALW_INLINE INLINE void OsSemPrioLock(void)
{

}
OS_SEC_ALW_INLINE INLINE void OsSemPrioUnLock(void)
{
    
}

OS_SEC_ALW_INLINE INLINE void OsSemIfPrioLock(struct TagSemCb *sem)
{
   (void)sem;
}

OS_SEC_ALW_INLINE INLINE void OsSemIfPrioUnLock(struct TagSemCb *sem)
{
   (void)sem;
}
#endif

extern U32 OsSemCreate(U32 count, U32 semType, enum SemMode semMode, SemHandle *semHandle, U32 cookie);
extern bool OsSemBusy(SemHandle semHandle);

#endif /* PRT_SEM_EXTERNAL_H */
