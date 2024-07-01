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
 * Description: 信号量模块
 */
#include "prt_sem_external.h"

OS_SEC_BSS struct TagListObject g_unusedSemList;
OS_SEC_BSS struct TagSemCb *g_allSem;
#if defined(OS_OPTION_SMP)
OS_SEC_BSS volatile uintptr_t g_semPrioLock;
#endif
#if defined(OS_OPTION_SEM_PRIO_INHERIT)
extern U32 OsCheckPrioritySet(struct TagTskCb *taskCb, TskPrior taskPrio);
#endif

OS_SEC_L4_TEXT bool OsSemBusy(SemHandle semHandle)
{
    struct TagSemCb *semCb = NULL;

    semCb = GET_SEM(semHandle);
    if (GET_MUTEX_TYPE(semCb->semType) != SEM_MUTEX_TYPE_RECUR && semCb->semCount == 0 &&
        GET_SEM_TYPE(semCb->semType) == SEM_TYPE_BIN) {
        return TRUE;
    } else if (GET_MUTEX_TYPE(semCb->semType) == SEM_MUTEX_TYPE_RECUR && semCb->semCount == 0 &&
        semCb->semOwner != RUNNING_TASK->taskPid) {
        return TRUE;
    }

    return FALSE;
}

OS_SEC_L4_TEXT U32 OsSemRegister(const struct SemModInfo *modInfo)
{
    if (modInfo->maxNum == 0) {
        return OS_ERRNO_SEM_REG_ERROR;
    }

    g_maxSem = modInfo->maxNum;

    return OS_OK;
}
OS_SEC_ALW_INLINE INLINE U32 OsSemInitCb(void)
{
    struct TagSemCb *semNode = NULL;
    U32 idx;

    /* g_maxSem在注册时已判断是否大于0，这里不需判断 */
    g_allSem = (struct TagSemCb *)OsMemAllocAlign((U32)OS_MID_SEM, OS_MEM_DEFAULT_FSC_PT,
                                                  g_maxSem * sizeof(struct TagSemCb),
                                                  MEM_ADDR_ALIGN_004);
    if (g_allSem == NULL) {
        return OS_ERRNO_SEM_NO_MEMORY;
    }
    if (memset_s((void *)g_allSem, g_maxSem * sizeof(struct TagSemCb), 0, g_maxSem * sizeof(struct TagSemCb)) !=
        EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    OS_LIST_INIT(&g_unusedSemList);
    for (idx = 0; idx < g_maxSem; idx++) {
        semNode = ((struct TagSemCb *)g_allSem) + idx;
        semNode->semId = (U16)idx;
#if defined(OS_OPTION_SMP)
        OsSpinLockInitInner(&semNode->semLock);
#endif
        ListTailAdd(&semNode->semList, &g_unusedSemList);
    }

    return OS_OK;
}
/*
 * 描述：信号量初始化
 */
OS_SEC_L4_TEXT U32 OsSemInit(void)
{
    U32 ret = OsSemInitCb();

#if defined(OS_OPTION_SMP)
    OsSpinLockInitInner(&g_semPrioLock);
#endif

#if defined(OS_OPTION_SEM_PRIO_INHERIT)
    g_checkPrioritySet = OsCheckPrioritySet;
#endif
    return ret;
}

OS_SEC_ALW_INLINE INLINE void OsSemCreateCbInit(U32 count, U32 semType, enum SemMode semMode,
                                           struct TagSemCb *semCreated)
{
    semCreated->semCount = count;
    semCreated->semStat = OS_SEM_USED;
    semCreated->semMode = semMode;
    semCreated->semType = semType;
    semCreated->semOwner = OS_INVALID_OWNER_ID;
    if (GET_SEM_TYPE(semType) == SEM_TYPE_BIN) {
        OS_LIST_INIT(&semCreated->semBList);
#if defined(OS_OPTION_SEM_RECUR_PV)
        semCreated->recurCount = 0;
#endif
    }

    OS_LIST_INIT(&semCreated->semList);
}
OS_SEC_ALW_INLINE INLINE U32 OsSemCreateCb(U32 count, U32 semType, enum SemMode semMode,
                                           SemHandle *semHandle, U32 cookie)
{
    struct TagSemCb *semCreated = NULL;
    struct TagListObject *unusedSem = NULL;
    (void)cookie;

    if (ListEmpty(&g_unusedSemList)) {
        return OS_ERRNO_SEM_ALL_BUSY;
    }

    /* 在空闲链表中取走一个控制节点 */
    unusedSem = OS_LIST_FIRST(&(g_unusedSemList));
    ListDelete(unusedSem);

    /* 获取到空闲节点对应的信号量控制块，并开始填充控制块 */
    semCreated = (GET_SEM_LIST(unusedSem));

    OsSemCreateCbInit(count, semType, semMode, semCreated);

    *semHandle = (SemHandle)semCreated->semId;

    return OS_OK;
}

/*
 * 描述：创建一个信号量
 */
OS_SEC_L4_TEXT U32 OsSemCreate(U32 count, U32 semType, enum SemMode semMode,
                               SemHandle *semHandle, U32 cookie)
{
    uintptr_t intSave;
    U32 ret;

    if (semHandle == NULL) {
        return OS_ERRNO_SEM_PTR_NULL;
    }

    SEM_INIT_IRQ_LOCK(intSave);

    ret = OsSemCreateCb(count, semType, semMode, semHandle, cookie);
    if (ret != OS_OK) {
        SEM_INIT_IRQ_UNLOCK(intSave);
        return ret;
    }

    SEM_INIT_IRQ_UNLOCK(intSave);
    return OS_OK;
}

/*
 * 描述：创建一个信号量
 */
OS_SEC_L4_TEXT U32 PRT_SemCreate(U32 count, SemHandle *semHandle)
{
    U32 ret;

    if (count > OS_SEM_COUNT_MAX) {
        return OS_ERRNO_SEM_OVERFLOW;
    }

    ret = OsSemCreate(count, SEM_TYPE_COUNT, SEM_MODE_FIFO, semHandle, (U32)(uintptr_t)semHandle);
    return ret;
}

#if defined(OS_OPTION_BIN_SEM)
/*
 * 描述：创建一个互斥信号量，支持优先级继承
 */
OS_SEC_L4_TEXT U32 PRT_SemMutexCreate(SemHandle *semHandle)
{
    return OsSemCreate(OS_SEM_FULL, SEM_TYPE_BIN | (SEM_MUTEX_TYPE_RECUR << 4), SEM_MODE_PRIOR, semHandle,
        (U32)(uintptr_t)semHandle);
}
#endif

/*
 * 描述：删除一个信号量
 */
OS_SEC_L4_TEXT U32 PRT_SemDelete(SemHandle semHandle)
{
    uintptr_t intSave;
    struct TagSemCb *semDeleted = NULL;

    if (semHandle >= (SemHandle)g_maxSem) {
        return OS_ERRNO_SEM_INVALID;
    }
    semDeleted = GET_SEM(semHandle);

    intSave = OsIntLock();

    SEM_CB_LOCK(semDeleted);

    if (semDeleted->semStat == OS_SEM_UNUSED) {
        SEM_CB_IRQ_UNLOCK(semDeleted, intSave);
        return OS_ERRNO_SEM_INVALID;
    }
    if (!ListEmpty(&semDeleted->semList)) {
        SEM_CB_IRQ_UNLOCK(semDeleted, intSave);
        return OS_ERRNO_SEM_PENDED;
    }
#ifdef OS_OPTION_BIN_SEM
    if ((semDeleted->semOwner != OS_INVALID_OWNER_ID) && (GET_SEM_TYPE(semDeleted->semType) == SEM_TYPE_BIN)) {
        SEM_CB_IRQ_UNLOCK(semDeleted, intSave);
        return OS_ERRNO_SEM_MUTEX_HOLDING;
    }
#endif
    semDeleted->semStat = OS_SEM_UNUSED;
    SEM_CB_UNLOCK(semDeleted);
    SEM_INIT_LOCK();
    ListAdd(&semDeleted->semList, &g_unusedSemList);
    SEM_INIT_UNLOCK();

    OsIntRestore(intSave);
    return OS_OK;
}
