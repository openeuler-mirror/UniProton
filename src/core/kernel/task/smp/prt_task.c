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
 * Description: Task模块
 */
#include "prt_task_external.h"
#include "prt_task_internal.h"
#include "prt_smp_task_internal.h"


#if defined(OS_OPTION_POWEROFF)
bool g_sysPowerOffFlag = false;

OS_SEC_TEXT void OsPowerOffSetFlag(void)
{
    g_sysPowerOffFlag = true;
}
OS_SEC_TEXT void OsPowerOffFuncHook(PowerOffFuncT powerOffFunc)
{
    g_sysPowerOffHook = powerOffFunc;
}

OS_SEC_TEXT void SetOfflineFlagDefaultFunc(PowerOffFuncT powerOffFunc)
{
    return;
}

OS_SEC_TEXT void OsSetOfflineFlagHook(SetOfflineFlagFuncT setOfflineFlagFunc)
{
    g_setOfflineFlagHook = setOfflineFlagFunc;
}

#endif
/*
 * 描述：Idle背景任务
 */
OS_SEC_L2_TEXT void OsIdleTaskExe(void) {
    OsVoidFunc coreSleep;

    OS_MHOOK_ACTIVATE_PARA0(OS_HOOK_IDLE_PERIOD);

#if defined(OS_OPTION_POWEROFF)
        if (g_sysPowerOffHook != NULL && g_sysPowerOffFlag) {
            U32 coreId = OsGetCoreID();
            if (coreId == g_cfgPrimaryCore) {
                g_sysPowerOffHook();
            }
        }
#endif

    coreSleep = g_taskCoreSleep;
    if (coreSleep != NULL) {
        coreSleep();
    }
}
OS_SEC_L2_TEXT void OsTskIdleBgd(void)
{
    OS_SHOOK_ACTIVATE_PARA0(OS_HOOK_IDLE_PREFIX);

    while (1) {
        OsIdleTaskExe();
    }
    
}


OS_SEC_ALW_INLINE INLINE void OsTskReadyAddOnly(struct TagTskCb *task)
{
    struct TagOsRunQue *rq = GET_RUNQ(task->coreID);
    TSK_STATUS_SET(task, OS_TSK_READY);

    if(UNLIKELY(task->isOnRq)){
        return;
    }
    OsEnqueueTask(rq, task, 0);

    OsIncNrRunning(rq);

    return;
}
/*
 * 描述：将任务添加到就绪队列, 调用者确保不会换核，并锁上rq
 */
OS_SEC_L0_TEXT void OsTskReadyAdd(struct TagTskCb *task)
{
    OsTskReadyAddOnly(task);

    OsReschedTask(task);

    return;
}
#if (defined(OS_OPTION_SMP) && (OS_MAX_CORE_NUM > 1))
OS_SEC_L0_TEXT void OsTskReadyAddNoWakeUpIpc(struct TagTskCb *task)
{
    OsTskReadyAddOnly(task);
    OsReschedTaskNoWakeIpc(task);
    return;
}
#endif
/*
 * 描述：将任务添加到就绪链表头，关中断外部保证
 */
OS_SEC_L0_TEXT void OsTskReadyDel(struct TagTskCb *taskCb)
{
    struct TagOsRunQue *runQue = NULL;
    TSK_STATUS_CLEAR(taskCb, OS_TSK_READY);

    runQue = GET_RUNQ(taskCb->coreID);

    if(!(taskCb->isOnRq)) {
        return;
    }

    OsDequeueTask(runQue, taskCb, 0);
    OsDecNrRunning(runQue);

    if (taskCb == runQue->tskCurr) {
        OsReschedTask(taskCb);
    }

    return;
}

/*
 * 描述：添加任务到超时链表
 */
OS_SEC_L0_TEXT void OsTskTimerAdd(struct TagTskCb *taskCb, uintptr_t timeout)
{
    struct TagTskCb *tskDelay = NULL;
    struct TagOsTskSortedDelayList *tskDlyBase = NULL;
    struct TagListObject *taskList = NULL;

    OS_SET_DLYBASE_AND_TSK_CORE(tskDlyBase, taskCb);

    taskCb->expirationTick = g_uniTicks + timeout;
    taskList = &tskDlyBase->tskList;

    CPU_OVERTIME_SORT_LIST_LOCK(tskDlyBase);

    if (ListEmpty(taskList)) {
        ListTailAdd(&taskCb->timerList, taskList);
    } else {
        /* Put the task to right location */
        LIST_FOR_EACH(tskDelay, taskList, struct TagTskCb, timerList) {
            if (tskDelay->expirationTick > taskCb->expirationTick) {
                break;
            }
        }

        ListTailAdd(&taskCb->timerList, &tskDelay->timerList);
    }
    OsTskDlyNearestTicksRefresh(tskDlyBase);

    CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase);

    return;
}


