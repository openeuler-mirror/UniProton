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
#include "prt_asm_cpu_external.h"
#include "prt_tick_external.h"

OS_SEC_BSS struct TagOsTskSortedDelayList g_tskSortedDelay;
OS_SEC_BSS struct TagOsRunQue g_runQueue;  // 核的局部运行队列

/*
 * 描述：任务调度，切换到最高优先级任务
 */
OS_SEC_TEXT void OsTskSchedule(void)
{
    /* 外层已经关中断 */
    /* Find the highest task */
    OsTskHighestSet();

    /* In case that running is not highest then reschedule */
    if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        /* only if there is not HWI or TICK the trap */
        if (OS_INT_INACTIVE) {
            OsTaskTrap();
            return;
        }
    }

    return;
}

OS_SEC_TEXT void OsTskScheduleFast(void)
{
    /* Find the highest task */
    OsTskHighestSet();

    /* In case that running is not highest then reschedule */
    if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        /* only if there is not HWI or TICK the trap */
        if (OS_INT_INACTIVE) {
            OsTaskTrapFast();
        }
    }
}

/*
 * 描述：如果快速切换后只有中断恢复，使用此接口
 */
OS_SEC_TEXT void OsTskScheduleFastPs(uintptr_t intSave)
{
    /* Find the highest task */
    OsTskHighestSet();

    /* In case that running is not highest then reschedule */
    if ((g_highestTask != RUNNING_TASK) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;

        /* only if there is not HWI or TICK the trap */
        if (OS_INT_INACTIVE) {
            OsTaskTrapFastPs(intSave);
        }
    }
}

OS_SEC_TEXT void OsTaskScan(void)
{
    struct TagTskCb *taskCb = NULL;
    bool needSchedule = FALSE;
    struct TagListObject *tskSortedDelayList = &g_tskSortedDelay.tskList;

    LIST_FOR_EACH(taskCb, tskSortedDelayList, struct TagTskCb, timerList) {
        if (taskCb->expirationTick > g_uniTicks) {
            break;
        }

        ListDelete(&taskCb->timerList);

        if ((OS_TSK_PEND & taskCb->taskStatus) != 0) {
            TSK_STATUS_CLEAR(taskCb, OS_TSK_PEND);
            ListDelete(&taskCb->pendList);
            taskCb->taskPend = NULL;
        } else if ((OS_TSK_EVENT_PEND & taskCb->taskStatus) != 0) {
            TSK_STATUS_CLEAR(taskCb, OS_TSK_EVENT_PEND);
        } else if ((OS_TSK_QUEUE_PEND & taskCb->taskStatus) != 0) {
            ListDelete(&taskCb->pendList);
            TSK_STATUS_CLEAR(taskCb, OS_TSK_QUEUE_PEND);
        } else if ((OS_TSK_RW_PEND & taskCb->taskStatus) != 0) {
            ListDelete(&taskCb->pendList);
            TSK_STATUS_CLEAR(taskCb, OS_TSK_RW_PEND);
        } else if ((OS_TSK_DELAY_INTERRUPTIBLE & taskCb->taskStatus) != 0) {
            TSK_STATUS_CLEAR(taskCb, OS_TSK_DELAY_INTERRUPTIBLE);
#if defined(OS_OPTION_LINUX)
            KTHREAD_TSK_STATE_SET(taskCb, TASK_RUNNING);
#endif
        } else {
            TSK_STATUS_CLEAR(taskCb, OS_TSK_DELAY);
        }

        if ((OS_TSK_SUSPEND & taskCb->taskStatus) == 0) {
            OsTskReadyAddBgd(taskCb);
            needSchedule = TRUE;
        }

        taskCb = LIST_COMPONENT(tskSortedDelayList, struct TagTskCb, timerList);
    }

    if (needSchedule) {
        OsTskScheduleFast();
    }
}

#if defined(OS_OPTION_TICKLESS)
/*
 * 描述：获取任务延时排序链的最近到期 tick（绝对值）。
 * AMP 任务延时链 g_tskSortedDelay.tskList 按 expirationTick 升序，头节点即最近到期。
 * 注册到 g_getTskDlyNearestTick，让 task delay 进入 tickless sleep budget；否则 idle
 * 只看到 swtmr/cpup 事件，会按其 budget 补偿，导致 task delay 被推迟、g_uniTicks 虚高。
 * 对位 SMP 的 OsTskDlyNearestTickGet（SMP 读维护好的 nearestTicks，AMP 读有序链头节点）。
 */
OS_SEC_TEXT U64 OsAmpTskDlyNearestTickGet(U32 coreID)
{
    (void)coreID;
    struct TagListObject *list = &g_tskSortedDelay.tskList;
    U64 ticks = OS_TICKLESS_FOREVER;
    uintptr_t intSave = OsIntLock();

    if (list->next != list) {
        struct TagTskCb *head = LIST_FIRST_ENTITY(list, struct TagTskCb, timerList);
        ticks = head->expirationTick;
    }
    OsIntRestore(intSave);
    return ticks;
}
#endif

OS_SEC_L0_TEXT void OsSpinLockTaskRq(struct TagTskCb* taskCB)
{
    (void)taskCB;
}

OS_SEC_L0_TEXT struct TagOsRunQue *OsSpinLockRunTaskRq(void)
{
    return NULL;
}

OS_SEC_L0_TEXT void OsSpinUnlockTaskRq(struct TagTskCb* taskCB)
{
    (void)taskCB;
}

OS_SEC_L0_TEXT void OsSpinUnLockRunTaskRq(struct TagOsRunQue *runQue)
{
    (void)runQue;
}

OS_SEC_L0_TEXT U32 OsTryLockTaskOperating(U32 operate, struct TagTskCb *taskCB, uintptr_t *intSave)
{
    (void)operate;
    (void)taskCB;
    *intSave = OsIntLock();
    return OS_OK;
}