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
 * Description: Task模块
 */
#include "prt_task_external.h"
#include "prt_task_internal.h"
#include "prt_amp_task_internal.h"

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
OS_SEC_TEXT void OsTskIdleBgd(void)
{
    TskCoresleep coreSleep = NULL;
    OS_SHOOK_ACTIVATE_PARA0(OS_HOOK_IDLE_PREFIX);

    while (TRUE) {
        OS_MHOOK_ACTIVATE_PARA0(OS_HOOK_IDLE_PERIOD);

#if defined(OS_OPTION_POWEROFF)
        if (g_sysPowerOffHook != NULL && g_sysPowerOffFlag) {
            g_sysPowerOffHook();
        }
#endif
        /* 防止g_taskCoreSleep中间被修改后，判空无效 */
        coreSleep = g_taskCoreSleep;
        if (coreSleep != NULL) {
            coreSleep();
        }
    }
}

/*
 * 描述：将任务添加到就绪队列, 调用者确保不会换核，并锁上rq
 */
OS_SEC_L0_TEXT void OsTskReadyAdd(struct TagTskCb *task)
{
    struct TagOsRunQue *rq = &g_runQueue;
    TSK_STATUS_SET(task, OS_TSK_READY);

    OS_TSK_EN_QUE(rq, task, 0);
    OsTskHighestSet();

    return;
}

/*
 * 描述：将任务添加到就绪链表头，关中断外部保证
 */
OS_SEC_L0_TEXT void OsTskReadyDel(struct TagTskCb *taskCb)
{
    struct TagOsRunQue *runQue = &g_runQueue;
    TSK_STATUS_CLEAR(taskCb, OS_TSK_READY);

    OS_TSK_DE_QUE(runQue, taskCb, 0);
    OsTskHighestSet();

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

    tskDlyBase = &g_tskSortedDelay;

    taskCb->expirationTick = g_uniTicks + timeout;
    taskList = &tskDlyBase->tskList;

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

    return;
}
