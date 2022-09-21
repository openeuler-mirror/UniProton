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
 * Create: 2022-09-21
 * Description: 网络
 */
#include <arch/sys_arch.h>
#include <lwip/sys.h>
#include <lwip/debug.h>
#include "prt_config.h"
#include "prt_task.h"
#include "prt_queue.h"
#include "prt_sem.h"
#include "prt_config.h"
#include "prt_sys.h"
#include "prt_tick_external.h"
#include "prt_task_external.h"
#include "prt_clk.h"
#include "prt_queue_external.h"

#if (OS_MAX_CORE_NUM > 1)
OS_SPIN_LOCK_INIT(g_archProtectSpin);
static TskHandle g_lwprotThread = OS_ERRNO_TSK_ID_INVALID;
static S32 g_lwprotCount = 0;
#endif /* OS_MAX_CORE_NUM */

#define LWIP_LOG_BUF_SIZE 128

#define ROUND_UP_DIV(val, div) ((U64)(((U64)(val) + (U64)(div) - 1) / (U64)(div)))

/**
 * Thread and System misc
 */
sys_thread_t sys_thread_new(const char *name, lwip_thread_fn thread, void *arg, int stackSize, int prio)
{
    TskHandle taskId = OS_ERRNO_TSK_ID_INVALID;
    U32 ret;
    struct TskInitParam task = {0};

    if (name == NULL || (strlen(name) == 0)) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_thread_new: name is null"));
        return -1;
    }

    /* Create host Task */
    task.taskEntry = (TskEntryFunc)thread;
    task.stackSize = (U32)stackSize;
    task.name = (char *)name;
    task.taskPrio = (TskPrior)prio;
    task.stackAddr = 0;
    ret = PRT_TaskCreate(&taskId, &task);
    if (ret != OS_OK) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_thread_new: PRT_TaskCreate error %u\n", ret));
        return ret;
    }

    ret = PRT_TaskResume(taskId);
    if (ret != OS_OK) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_thread_new: PRT_TaskResume error %u\n", ret));
        return ret;
    }
    return taskId;
}

void sys_init(void)
{
    U32 seedlsb = (U32)PRT_ClkGetCycleCount64();
    srand(seedlsb);
}

u32_t sys_now(void)
{
    return (U32)(((U32)PRT_TickGetCount() * OS_SYS_MS_PER_SECOND) / OS_TICK_PER_SECOND);
}

/**
 * Protector
 */
sys_prot_t sys_arch_protect(void)
{
#if (OS_MAX_CORE_NUM > 1)
    /* Note that we are using spinlock instead of mutex for LiteOS-SMP here:
     * 1. spinlock is more effective for short critical region protection.
     * 2. this function is called only in task context, not in interrupt handler.
     *    so it's not needed to disable interrupt.
     */
    if (g_lwprotThread != RUNNING_TASK->taskPid) {
        /* We are locking the spinlock where it has not been locked before
         * or is being locked by another thread */
        g_lwprotThread = RUNNING_TASK->taskPid;
        g_lwprotCount = 1;
    } else {
        /* It is already locked by THIS thread */
        g_lwprotCount++;
    }
#else
    PRT_TaskLock();

#endif /* OS_MAX_CORE_NUM */
    return 0; /* return value is unused */
}

void sys_arch_unprotect(sys_prot_t pval)
{
    LWIP_UNUSED_ARG(pval);
#if (OS_MAX_CORE_NUM > 1)
    if (g_lwprotThread == RUNNING_TASK->taskPid) {
        g_lwprotCount--;
        if (g_lwprotCount == 0) {
            g_lwprotThread = OS_ERRNO_TSK_ID_INVALID;
        }
    }
#else

    PRT_TaskUnlock();
#endif /* OS_MAX_CORE_NUM */
}

/**
 * MessageBox
 */
err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_new: mbox is null"));
        return ERR_ARG;
    }

    char qName[] = "lwIP";
    U32 ret = PRT_QueueCreate((U16)size, sizeof(void *), (U32 *)mbox);
    switch (ret) {
        case OS_OK:
            return ERR_OK;
        case OS_ERRNO_QUEUE_CB_UNAVAILABLE:
        case OS_ERRNO_QUEUE_CREATE_NO_MEMORY:
            return ERR_MEM;
        default:
            break;
    }

    LWIP_DEBUGF(SYS_DEBUG, ("%s: PRT_QueueCreate error %u\n", __FUNCTION__, ret));
    return ERR_ARG;
}

void sys_mbox_post(sys_mbox_t *mbox, void *msg)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_post: mbox is null"));
        return;
    }

    /* Caution: the second parameter is NOT &msg */
    enum QueuePrio prio = OS_QUEUE_NORMAL;
    U32 ret = PRT_QueueWrite((U32)(*mbox), (void*)&msg, sizeof(char *), OS_WAIT_FOREVER, (U32)prio);
    if (ret != OS_OK) {
        LWIP_DEBUGF(SYS_DEBUG, ("%s: PRT_QueueWrite error %u\n", __FUNCTION__, ret));
    }
}

err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_trypost: mbox is null"));
        return ERR_ARG;
    }

    /* Caution: the second parameter is NOT &msg */
    enum QueuePrio prio = OS_QUEUE_NORMAL;
    U32 ret = PRT_QueueWrite((U32)(*mbox), (void*)&msg, sizeof(char *), 0, (U32)prio);
    switch (ret) {
        case OS_OK:
            return ERR_OK;
        case OS_ERRNO_QUEUE_NO_SOURCE:
            return ERR_MEM;
        default:
            break;
    }
    LWIP_DEBUGF(SYS_DEBUG, ("%s: PRT_QueueWrite error %u\n", __FUNCTION__, ret));
    return ERR_ARG;
}

err_t sys_mbox_trypost_fromisr(sys_mbox_t *mbox, void *msg)
{
    (void)mbox;
    (void)msg;
    return ERR_ARG;
}

u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeoutMs)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_arch_mbox_fetch: mbox is null"));
        return  (u32_t)ERR_ARG;
    }
    void *ignore = 0; /* if msg == NULL, the fetched msg should be dropped */
    U64 tick = ROUND_UP_DIV((U64)timeoutMs * OS_TICK_PER_SECOND, OS_SYS_MS_PER_SECOND);
    U32 ret = PRT_QueueRead((U32)(*mbox), msg ? msg : &ignore, sizeof(void *), tick ? (U32)tick : OS_WAIT_FOREVER);
    switch (ret) {
        case OS_OK:
            return ERR_OK;
        case OS_ERRNO_QUEUE_NO_SOURCE:
        case OS_ERRNO_QUEUE_TIMEOUT:
            return SYS_ARCH_TIMEOUT;
        default:
            break;
    }

    LWIP_DEBUGF(SYS_DEBUG, ("%s: PRT_QueueRead error 0x%x\n", __FUNCTION__, ret));
    return SYS_ARCH_TIMEOUT; /* Errors should be treated as timeout */
}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_arch_mbox_tryfetch: mbox is null"));
        return  (u32_t)ERR_ARG;
    }

    void *ignore = 0; /* if msg==NULL, the fetched msg should be dropped */
    U32 ret = PRT_QueueRead((U32)*mbox, msg ? msg : &ignore, sizeof(void *), 0);
    switch (ret) {
        case OS_OK:
            return ERR_OK;
        case OS_ERRNO_QUEUE_NO_SOURCE:
            return SYS_MBOX_EMPTY;
        case OS_ERRNO_QUEUE_TIMEOUT:
            return SYS_ARCH_TIMEOUT;
        default:
            break;
    }
    LWIP_DEBUGF(SYS_DEBUG, ("%s: PRT_QueueRead error %u\n", __FUNCTION__, ret));
    return SYS_MBOX_EMPTY; /* Errors should be treated as timeout */
}

void sys_mbox_free(sys_mbox_t *mbox)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_free: mbox is null"));
        return;
    }
    (void)PRT_QueueDelete(*mbox);
}

int sys_mbox_valid(sys_mbox_t *mbox)
{
    if (*mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_valid: mbox is null"));
        return ERR_ARG;
    }
    U32 innerId = OS_QUEUE_INNER_ID(*mbox);
    struct TagQueCb *queueCb = NULL;
    queueCb = (struct TagQueCb *)GET_QUEUE_HANDLE(innerId);
    return queueCb->queueState == OS_QUEUE_USED;
}

void sys_mbox_set_invalid(sys_mbox_t *mbox)
{
    if (mbox == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mbox_set_invalid: mbox is null"));
        return;
    }
    *mbox = OS_QUEUE_MAX_SUPPORT_NUM;
}

/**
 * Semaphore
 */
err_t sys_sem_new(sys_sem_t *sem, u8_t count)
{
    if (sem == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_sem_new: sem is null"));
        return ERR_ARG;
    }

    U32 ret = PRT_SemCreate((U32)count, (SemHandle*)sem);
    if (ret != OS_OK) {
        return ERR_ARG;
    }

    return ERR_OK;
}

void sys_sem_signal(sys_sem_t *sem)
{
    if (sem == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_sem_signal: sem is null"));
        return;
    }
    (void)PRT_SemPost((SemHandle)(*sem));
}

u32_t sys_arch_sem_wait(sys_sem_t *sem, u32_t timeoutMs)
{
    if (sem == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_arch_sem_wait: sem is null"));
        return  (u32_t)ERR_ARG;
    }
    U64 tick = ROUND_UP_DIV((U64)timeoutMs * OS_TICK_PER_SECOND, OS_SYS_MS_PER_SECOND);
    U32 ret = PRT_SemPend((SemHandle)(*sem), tick ? (U32)tick : OS_WAIT_FOREVER);
    switch (ret) {
        case OS_OK:
            return ERR_OK;
        case OS_ERRNO_SEM_TIMEOUT:
            return SYS_ARCH_TIMEOUT;
        default:
            break;
    }
    LWIP_DEBUGF(SYS_DEBUG, ("%s: PRT_SemPend error %u\n", __FUNCTION__, ret));
    return SYS_ARCH_TIMEOUT; /* Errors should be treated as timeout */
}

void sys_sem_free(sys_sem_t *sem)
{
    if (sem == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_sem_free: sem is null"));
        return;
    }
    (void)PRT_SemDelete((SemHandle)(*sem));
}

int sys_sem_valid(sys_sem_t *sem)
{
    if (sem == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_sem_valid: sem is null"));
        return ERR_ARG;
    }
    return (SemHandle)(*sem) != OS_SEM_MAX_SUPPORT_NUM;
}

void sys_sem_set_invalid(sys_sem_t *sem)
{
    if (sem == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_sem_set_invalid: sem is null"));
        return;
    }
    *sem = OS_SEM_MAX_SUPPORT_NUM;
}

/**
 * Mutex
 */
err_t sys_mutex_new(sys_mutex_t *mutex)
{
    if (mutex == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_sem_new: sem is null"));
        return ERR_ARG;
    }

    U32 ret = PRT_SemCreate(1, (SemHandle*)(mutex));
    if (ret != OS_OK) {
        return ERR_ARG;
    }

    return ERR_OK;
}

void sys_mutex_lock(sys_mutex_t *mutex)
{
    if (mutex == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mutex_lock: mutex is null"));
        return;
    }

    (void)PRT_SemPend((SemHandle)(*mutex), OS_WAIT_FOREVER);
}

void sys_mutex_unlock(sys_mutex_t *mutex)
{
    if (mutex == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mutex_unlock: mutex is null"));
        return;
    }

    (void)PRT_SemPost((SemHandle)(*mutex));
}

void sys_mutex_free(sys_mutex_t *mutex)
{
    if (mutex == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mutex_free: mutex is null"));
        return;
    }

    (void)PRT_SemDelete((SemHandle)(*mutex));
}

int sys_mutex_valid(sys_mutex_t *mutex)
{
    if (mutex == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mutex_valid: mutex is null"));
        return ERR_ARG;
    }

    return *mutex != OS_SEM_MAX_SUPPORT_NUM;
}

void sys_mutex_set_invalid(sys_mutex_t *mutex)
{
    if (mutex == NULL) {
        LWIP_DEBUGF(SYS_DEBUG, ("sys_mutex_set_invalid: mutex is null"));
        return;
    }

    *mutex = OS_SEM_MAX_SUPPORT_NUM;
}

void OsLwipLogPrintf(const char *fmt, ...)
{
    if ((fmt == NULL) || (strlen(fmt) == 0)) {
        return;
    }

    int len;
    char buf[LWIP_LOG_BUF_SIZE] = {0};
    va_list ap;
    va_start(ap, fmt);
    len = vsprintf_s(buf, sizeof(buf) - 1, fmt, ap);
    va_end(ap);
    if (len < 0) {
        LWIP_LOGGER("log param invalid or buf is not enough.");
        return;
    }

    printf("%s\n\r", buf);
    LWIP_LOGGER(buf);
}
