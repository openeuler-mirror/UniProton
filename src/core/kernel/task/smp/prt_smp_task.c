/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-26
 * Description: Task 模块
 */
#include "prt_task_internal.h"
#include "prt_sem_external.h"
#include "prt_task_sched_external.h"
#include "prt_smp_task_internal.h"
#include "prt_cpu_external.h"
#include "../../../include/uapi/hw/armv8/os_atomic_armv8.h"

OS_SEC_BSS struct TagOsTskSortedDelayList g_tskSortedDelay[OS_MAX_CORE_NUM];

/*
* 将任务添加到就绪列表
*/
OS_SEC_L0_TEXT void OsTskReadyAddBgd(struct TagTskCb *task)
{
    OsTskReadyAdd(task);
}
/*
 * 描述：更新指定核任务延时链上最近需要触发的tick刻度
 * 输入：coreID --- 指定核
 * 返回：NA
 */
OS_SEC_L0_TEXT void OsTskDlyNearestTicksRefresh(struct TagOsTskSortedDelayList *tskDlyBase)
{
    U64 ticks = OS_TICKLESS_FOREVER;

    struct TagListObject *tskList = &tskDlyBase->tskList;

    if (!ListEmpty(tskList)) {
        struct TagTskCb *taskCB = LIST_COMPONENT(OS_LIST_FIRST(tskList), struct TagTskCb, timerList);

        ticks = taskCB->expirationTick;
    }
    tskDlyBase->nearestTicks = ticks;
}

#if !defined(OS_OPTION_TASK_AFFINITY_STATIC)
/*
 * 描述：将running任务添加到其他核上
 * 输入：taskCB             -- 任务控制块
 *      destRunQue         -- 目的RQ
 * 返回：NA
 */
OS_SEC_TEXT void OsRunningReadyAdd(struct TagTskCb *taskCB)
{
    OsSpinLockTaskRq(taskCB);
    TSK_STATUS_CLEAR(taskCB, OS_TSK_RUNNING);
    if(!(taskCB->taskStatus & OS_TSK_BLOCK)) {
        OsTskReadyAddBgd(taskCB);
    }
    OS_TSK_OP_CLR(taskCB, OS_TSK_OP_MIGRATING);

    OsSpinUnlockTaskRq(taskCB);
    return;
}
#endif

#if (!defined(OS_OPTION_TASK_AFFINITY_STATIC) || defined(OS_OPTION_TASK_DELETE) || defined(OS_OPTION_TASK_SUSPEND))
#if (OS_MAX_CORE_NUM > 1)
OS_SEC_ALW_INLINE INLINE void OsTskReadyDelSyncWait(uintptr_t *intSave, U32 intHandshakeCount, U32 coreID)
{
    OsIntRestore(*intSave);

    while (1) {
        if((intHandshakeCount != *(volatile U32 *)&OS_SMP_HANDSHAKE_COUNT(coreID))||
            ((*(volatile U32*)&GET_RUNQ(coreID)->uniFlag & OS_FLG_HWI_ACTIVE) && (GET_RUNQ(coreID)->uniTaskLock == 0))) {
            break;
        }
        ASM_NOP();
    }
    *intSave = OsIntLock();
}
#else
OS_SEC_ALW_INLINE INLINE void OsTskReadyDelSyncWait(uintptr_t *intSave, U32 intHandshakeCount, U32 coreID)
{
    (void)intSave;
    (void)intHandshakeCount;
    (void)coreID;
}
#endif

/*
 * 描述：有一种就绪删除叫同步任务删除（需等待，因为任务时running的）
 * 输入：taskCB             -- 任务控制块
 * 返回：NA
 */
OS_SEC_L2_TEXT void OsTskReadyDelSync(struct TagTskCb *taskCB, uintptr_t *intSave)
{
    U32 intHandshakeCount = 0;
    struct TagOsRunQue *runQue = NULL;
    bool deQueNeedWait = FALSE;

    runQue = GET_RUNQ(taskCB->coreID);
    TSK_STATUS_CLEAR(taskCB, OS_TSK_READY);
    if((taskCB->taskStatus & OS_TSK_RUNNING) || (taskCB == runQue->tskCurr)) {
        deQueNeedWait = TRUE;
        OsDeactiveTask(runQue, taskCB, 0);
        runQue->needReschedule = TRUE;
        PRT_DMB();
        intHandshakeCount = OS_SMP_HANDSHAKE_COUNT(taskCB->coreID);
        OsSmpSendReschedule(runQue->rqCoreId);
    } else {
        OsDeactiveTask(runQue, taskCB, 0);
    }
    OsSpinUnlockTaskRq(taskCB);

    if (deQueNeedWait) {
        OsTskReadyDelSyncWait(intSave, intHandshakeCount, taskCB->coreID);
    }
}
#endif

#if !defined(OS_OPTION_TASK_AFFINITY_STATIC)
/*
 * 描述：任务绑定自己迁移
 * 输入：taskCB             -- 任务控制块
 *      coreMask            -- 目标核掩码
 * 输出：None
 * 返回：成功或者错误码
 */
static OS_SEC_L2_TEXT void OsTskBindSelf(struct TagTskCb *taskCB, U32 coreID, U32 coreMask)
{
    struct TagOsRunQue *srcRunQ = THIS_RUNQ();

    OsTskReadyDel(taskCB);

    OsTskSetCoreAllowed(coreMask, taskCB);
    if(((1UL << (taskCB->coreID)) & coreMask) == 0) {
        taskCB->scheClass = OS_SCHED_CLASS(coreID);
        TSK_CORE_SET(taskCB, coreID);
    }
    OsSplUnlock(&srcRunQ->spinLock);

    OsContextSave(taskCB);
    return;
}
#endif

#if (OS_MAX_CORE_NUM > 1)
#if (!defined(OS_OPTION_TASK_AFFINITY_STATIC) || defined(OS_OPTION_TASK_DELETE) || defined(OS_OPTION_TASK_SUSPEND))
/*
 * 描述：加锁清除当前任务的操作标记，该标记用于任务绑核过程中调用者被删除
        导致被绑任务无法加入到readyList中。必须在任务中并关中断后调用
 * 输入：tsk        ---     待操作的任务
 * 输出：None
 * 返回：TRUE
 *       FALSE
 */
OS_SEC_ALW_INLINE INLINE bool OsTskMarkBusy(struct TagTskCb *tsk)
{
    return PRT_AtomicCompareAndStore32(&tsk->opBusy, 0, 1);
}

/*
 * 描述：加锁清除当前任务的操作标记，该标记用于任务绑核过程中调用者被删除
        导致被绑任务无法加入到readyList中。
 * 输入：tsk        ---     待操作的任务
 * 输出：None
 * 返回：NONE
 */
OS_SEC_ALW_INLINE INLINE void OsTskUnmarkBusy(struct TagTskCb *tsk)
{
    (void)PRT_AtomicSwap32(&tsk->opBusy, 0);
}
OS_SEC_L2_TEXT U32 OsTaskOperaBegin(struct TagTskCb *runTask, struct TagTskCb *tgtTask)
{
    if (!OsTskMarkBusy(runTask)) {
        return OS_ERRNO_TSK_OPERATION_BUSY;
    }

    if (tgtTask != runTask) {
        if (!OsTskMarkBusy(tgtTask)) {
            OsTskUnmarkBusy(runTask);
            return OS_ERRNO_TSK_OPERATION_BUSY;
        }
    }
    return OS_OK;
}

OS_SEC_L2_TEXT void OsTaskOperaEnd(struct TagTskCb *runTask, struct TagTskCb *tgtTask)
{
    OsTskUnmarkBusy(runTask);
    if (runTask != tgtTask) {
        OsTskUnmarkBusy(tgtTask);
    }
}
#endif
#endif

OS_SEC_L2_TEXT U32 OsTaskSetAffinityCheck(TskHandle taskPID, U32 coreMask, struct TagTskCb **taskCB)
{
    struct TagTskCb *tempTaskCB = NULL;

    if (CHECK_TSK_PID_OVERFLOW(taskPID)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    if (TSK_GET_INDEX(taskPID) >= g_tskMaxNum) {
        return OS_ERRNO_TSK_OPERATE_IDLE;
    }

    tempTaskCB = GET_TCB_HANDLE(taskPID);

    if (TSK_IS_UNUSED(tempTaskCB)) {
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    if (coreMask == 0) {
        return OS_ERRNO_TSK_BIND_CORE_INVALID;
    }

    *taskCB = tempTaskCB;

    return OS_OK;
}

/*
 * 描述：任务绑定到核
 * 输入：taskCB             -- 任务控制块
 *      coreMask            -- 目标核掩码
 * 输出：None
 * 返回：成功或者错误码
 */
#if defined(OS_OPTION_TASK_AFFINITY_STATIC)
OS_SEC_L2_TEXT U32 OsTaskSetAffinity(TskHandle taskPID, OS_CORE_MASK coreMask)
{
    U32 ret;
    struct TagTskCb *taskCB = NULL;
    U32 coreID = OsGetRMB(coreMask);

    coreMask = CPUMASK_AND(coreMask, OS_ALLCORES_MASK);
    ret = OsTaskSetAffinityCheck(taskPID, coreMask, &taskCB);
    if (ret != OS_OK) {
        return ret;
    }

    if ((((1UL << g_numOfCores) - 1) & coreMask) == 0) {
        return OS_OK;
    }
    OsTskSetCoreAllowed(coreMask, taskCB);
    if (!CPUMASK_HAS_BIT(coreMask, taskCB->coreID)) {
        TSK_CORE_SET(taskCB, coreID);
    }
    return OS_OK;
}
#else
OS_SEC_ALW_INLINE INLINE U32 OsTaskSetAffinityPre(struct TagTskCb *runTask, struct TagTskCb *taskCB, uintptr_t *intSave)
{
    U32 ret;
    ret = OsTaskOperaBegin(runTask, taskCB);
    if (ret != OS_OK) {
        return ret;
    }

    ret = OsTryLockTaskOperating(OS_TSK_OP_MIGRATING, taskCB, intSave);
    if (ret != OS_OK) {
        OsTaskOperaEnd(runTask, taskCB);
        return ret;
    }

    if (OS_INT_ACTIVE && ((taskCB->taskStatus & OS_TSK_BLOCK) == 0)) {
        OS_TSK_OP_CLR(taskCB, OS_TSK_OP_MIGRATING);
        OsSpinUnlockTaskRq(taskCB);
        OsTaskOperaEnd(runTask, taskCB);
        OsIntRestore(*intSave);
        return OS_ERRNO_TSK_BIND_IN_HWI;
    }
    return OS_OK;
}

#define OS_COREMASK_AND(m1, m2) ((m1) & (m2))
#define OS_IS_CORE_IN_MASK(core, mask) OS_COREMASK_AND(((OS_CORE_MASK)(1) << (core)), (mask))
OS_SEC_ALW_INLINE INLINE U32 OsTskSetAffinitySelf(struct TagTskCb *taskCB, struct TagTskCb *runTask,
                                        struct TagOsRunQue *runQue, OS_CORE_MASK coreMask, U32 coreID)
{
    (void)runTask;
    if (!OS_IS_CORE_IN_MASK(THIS_CORE(), coreMask) && (OS_TASK_LOCK_DATA != 0)) {
        OS_TSK_OP_CLR(taskCB, OS_TSK_OP_MIGRATING);
        OsSplUnlock(&runQue->spinLock);
        return OS_ERRNO_TSK_BIND_SELF_WITH_TASKLOCK;
    }

    OsTskBindSelf(taskCB, coreID, coreMask);
    return OS_OK;
}
OS_SEC_L2_TEXT void OsTskBindStatusUpdt(struct TagTskCb *taskCB, struct TagTskCb *runTask,
                                        struct TagOsRunQue *runQue, OS_CORE_MASK coreMask, U32 coreID)
{
    OsTskSetCoreAllowed(coreMask, taskCB);
    if (!CPUMASK_HAS_BIT(coreMask, taskCB->coreID)) {
        taskCB->scheClass = OS_SCHED_CLASS(coreID);
        TSK_CORE_SET(taskCB, coreID);
    }
    OsSplUnlock(&runQue->spinLock);
    OsSpinLockTaskRq(taskCB);

    OS_TSK_OP_CLR(taskCB, OS_TSK_OP_MIGRATING);
    OsTaskOperaEnd(runTask, taskCB);
    return;
}
OS_SEC_L2_TEXT U32 OsTaskSetAffinity(TskHandle taskPID, OS_CORE_MASK coreMask)
{
    uintptr_t intSave = 0;
    struct TagTskCb *runTask = NULL;
    U32 ret;
    bool enqReady = FALSE;
    struct TagOsRunQue *runQue = NULL;
    U32 coreID = OsGetRMB(coreMask);
    struct TagTskCb *taskCB = NULL;
    coreMask = CPUMASK_AND(coreMask, OS_ALLCORES_MASK);
    ret = OsTaskSetAffinityCheck(taskPID, coreMask, &taskCB);
    if (ret != OS_OK) {
        return ret;
    }

    if((((1UL << g_numOfCores) - 1) & coreMask) == 0 ) {
        return OS_OK;
    }
    
    intSave = OsIntLock();
    runTask = RUNNING_TASK;
    OsIntRestore(intSave);

    ret = OsTaskSetAffinityPre(runTask, taskCB, &intSave);
    if (ret != OS_OK) {
        return ret;
    }
    runQue = GET_RUNQ(taskCB->coreID);

    if (taskCB == runTask) {
        OsTaskOperaEnd(runTask, taskCB);
        ret = OsTskSetAffinitySelf(taskCB, runTask, runQue, coreMask, coreID);
        OsIntRestore(intSave);
        return ret;
    }

    while ((volatile U32)(taskCB->taskStatus) & OS_TSK_READY) {
        enqReady = TRUE;
        OsTskReadyDelSync(taskCB, &intSave);

        OsSpinLockTaskRq(taskCB);
        runQue = GET_RUNQ(taskCB->coreID);
    }

    OsTskBindStatusUpdt(taskCB, runTask, runQue, coreMask, coreID);

    if(enqReady == FALSE) {
        OsSpinUnlockTaskRq(taskCB);
        OsIntRestore(intSave);
        return OS_OK;
    }
    OsMoveTaskToReady(taskCB);
    OsIntRestore(intSave);
    return OS_OK;
}
#endif

/*
 * 描述：任务绑定到核
 */
OS_SEC_L2_TEXT U32 PRT_TaskCoreBind(TskHandle taskPid, U32 coreMask)
{
    return OsTaskSetAffinity(taskPid, (OS_CORE_MASK)coreMask);
}

/*
 * 描述：任务绑定到核后解绑，恢复任务的可运行核位所有核
 */
OS_SEC_L2_TEXT U32 PRT_TaskCoreUnBind(TskHandle taskPid)
{
    return PRT_TaskCoreBind(taskPid, (U32)OS_ALLCORES_MASK);
}

OS_SEC_TEXT bool OsTskDlyScanNoPendLock(struct TagTskCb *taskCB, struct TagOsTskSortedDelayList *tskDlyBase)
{
    CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);
    OsSpinLockTaskRq(taskCB);
    CPU_OVERTIME_SORT_LIST_LOCK(tskDlyBase);
    if (((taskCB->taskStatus & OS_TSK_INUSE) != 0) && (tskDlyBase == CPU_TSK_DELAY_BASE(taskCB->timeCoreID)) &&
        ((U64)taskCB->expirationTick <= (U64)g_uniTicks) && (!ListEmpty(&taskCB->timerList)) &&
        (taskCB->taskSem == NULL)) {
        ListDelete(&taskCB->timerList);
        CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);
    } else {
        OsSpinUnlockTaskRq(taskCB);
        return TRUE;
    }
    return FALSE;
}

OS_SEC_TEXT bool OsTskDlyScanHasPendLock(struct TagTskCb *taskCB, volatile uintptr_t *pendedLock,
                                         struct TagOsTskSortedDelayList *tskDlyBase)
{
    CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);
    OsSplLock(pendedLock);
    OsSemPrioLock();
    OsSpinLockTaskRq(taskCB);
    CPU_OVERTIME_SORT_LIST_LOCK(tskDlyBase);
    if (((taskCB->taskStatus & OS_TSK_INUSE) != 0) && (tskDlyBase == CPU_TSK_DELAY_BASE(taskCB->timeCoreID)) &&
        ((U64)taskCB->expirationTick <= (U64)g_uniTicks) && (!ListEmpty(&taskCB->timerList)) && 
        (taskCB->taskSem == pendedLock)) {
        ListDelete(&taskCB->timerList);
        ListDelete(&taskCB->pendList);
        taskCB->taskSem = NULL;

        CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);
        OsSemPrioUnLock();
        OsSplUnlock(pendedLock);
    } else {
        OsSpinUnlockTaskRq(taskCB);
        OsSemPrioUnLock();
        OsSplUnlock(pendedLock);
        return TRUE;
    }
    return FALSE;
}

OS_SEC_TEXT bool OsTskDlyBaseListScan(struct TagListObject *tskList, struct TagOsTskSortedDelayList *tskDlyBase)
{
    struct TagTskCb *taskCB = NULL;
    volatile uintptr_t *pendedLock = NULL;

    taskCB = LIST_COMPONENT(OS_LIST_FIRST(tskList), struct TagTskCb, timerList);
    if ((U64)taskCB->expirationTick > (U64)g_uniTicks) {
        OsTskDlyNearestTicksRefresh(tskDlyBase);
        CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);
        return FALSE;
    }

    pendedLock = taskCB->taskSem;
    if (LIKELY(pendedLock == NULL)) {
        if (OsTskDlyScanNoPendLock(taskCB, tskDlyBase)) {
            return TRUE;
        }
    } else {
        if (OsTskDlyScanHasPendLock(taskCB, pendedLock, tskDlyBase)) {
            return TRUE;
        }
    }

    TSK_STATUS_CLEAR(taskCB, OS_TSK_PEND | OS_TSK_QUEUE_PEND | OS_TSK_EVENT_PEND| OS_TSK_DELAY);

    if(!(taskCB->taskStatus & (OS_TSK_SUSPEND_READY_BLOCK))) {
        OsTskReadyAddBgd(taskCB);
    }
    OsSpinUnlockTaskRq(taskCB);

    CPU_OVERTIME_SORT_LIST_LOCK(tskDlyBase);
    return TRUE;
}

/*
 * 描述：扫描任务延时链
 * 输入：tskDlyBase        ---     任务延时链
 * 输出：None
 * 返回：NONE
 */
OS_SEC_TEXT void OsTskDlyBaseScan(struct TagOsTskSortedDelayList *tskDlyBase)
{
    struct TagListObject *tskList = &(tskDlyBase->tskList);

    CPU_OVERTIME_SORT_LIST_LOCK(tskDlyBase);

    while (1) {
        if (ListEmpty(tskList)) {
            OsTskDlyNearestTicksRefresh(tskDlyBase);
            CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);
            break;
        } else {
            if (OsTskDlyBaseListScan(tskList, tskDlyBase)) {
                continue;
            } else {
                break;
            }
        }
    }
}

/*
 * 描述：任务延时扫描
 * 输入：None
 * 输出：None
 * 返回：NONE
 */
OS_SEC_TEXT void OsTaskScan(void)
{
    U32 thisCoreID = THIS_CORE();

    struct TagOsTskSortedDelayList *tskDlyBase = CPU_TSK_DELAY_BASE(thisCoreID);

    OsTskDlyBaseScan(tskDlyBase);
}

#if (OS_MAX_CORE_NUM > 1)
OS_SEC_L0_TEXT void OsSpinLockTaskRq(struct TagTskCb *taskCB)
{
    struct TagOsRunQue *runQue = NULL;

    while (1) {
        runQue = GET_RUNQ(taskCB->coreID);
        OsSplLock(&runQue->spinLock);
        if (LIKELY(taskCB->coreID == runQue->rqCoreId)) {
            break;
        } else {
            OsSplUnlock(&runQue->spinLock);
        }
    }
}
OS_SEC_L2_TEXT bool OsTrySpinLockTaskRq(struct TagTskCb *taskCB)
{
    struct TagOsRunQue *runQue = NULL;
    bool tryResult = FALSE;
    runQue = GET_RUNQ(taskCB->coreID);
    tryResult = OsSplTryLock(&runQue->spinLock);
    /* TRUE 表示加锁成功 */
    if (tryResult) {
        if (runQue->rqCoreId == taskCB->coreID) {
            return TRUE;
        }
        OsSplUnlock(&runQue->spinLock);
        return FALSE;
    }
    return FALSE;
}

OS_SEC_L0_TEXT struct TagOsRunQue *OsSpinLockRunTaskRq(void)
{
    struct TagOsRunQue *runQue = NULL;
    runQue = THIS_RUNQ();
    OsSplLock(&runQue->spinLock);
    return runQue;
}

OS_SEC_L0_TEXT void OsSpinUnLockRunTaskRq(struct TagOsRunQue *runQue)
{
    OsSplUnlock(&runQue->spinLock);
}
OS_SEC_L0_TEXT void OsSpinUnlockTaskRq(struct TagTskCb *taskCB)
{
    struct TagOsRunQue *runQue = NULL;
    runQue = GET_TASK_RQ(taskCB);
    OsSplUnlock(&runQue->spinLock);
}

OS_SEC_L2_TEXT U32 OsTryLockTaskOperating(U32 operate, struct TagTskCb *taskCB, uintptr_t *intSave)
{
    bool tryLockRq = FALSE;
    while (1) {
        if ((taskCB->taskOperating & OS_TSK_OP_DELETING) != 0) {
            return OS_ERRNO_TSK_NOT_CREATED;
        }
        if (taskCB->taskOperating != OS_TSK_OP_FREE) {
            continue;
        }
        *intSave = OsIntLock();
        tryLockRq = OsTrySpinLockTaskRq(taskCB);
        if (tryLockRq) {
            if (taskCB->taskOperating == OS_TSK_OP_FREE) {
                taskCB->taskOperating |= operate;
                break;
            } else if ((taskCB->taskOperating & OS_TSK_OP_DELETING) != 0) {
                OsSpinUnlockTaskRq(taskCB);
                OsIntRestore(*intSave);
                return OS_ERRNO_TSK_NOT_CREATED;
            } else {
                OsSpinUnlockTaskRq(taskCB);
                OsIntRestore(*intSave);
                continue;
            }
        }
        OsIntRestore(*intSave);
    }
    return OS_OK;
}
#endif