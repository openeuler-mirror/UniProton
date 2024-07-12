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
 * Description: 单核调度函数实现
 */
#include "prt_hook_external.h"
#include "prt_task_external.h"
#include "prt_irq_external.h"

#if defined(OS_OPTION_RR_SCHED)
/* 在中断尾部调用，更新当前任务的剩余时间片，并检查是否需要切换 */
OS_SEC_L0_TEXT void OsHwiEndCheckTimeSlice(U64 currTime)
{
    struct TagTskCb *currTsk = RUNNING_TASK;
    if (currTsk == NULL) {
        return;
    }
#if defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
    OsTimeSliceUpdate(currTsk, currTime, currTsk->irqUsedTime);
    currTsk->irqUsedTime = 0;
#else
    OsTimeSliceUpdate(currTsk, currTime, 0);
#endif
    if (currTsk->policy != OS_TSK_SCHED_RR) {
        return;
    }

    if (currTsk->timeSlice != 0) {
        return;
    }

    OsTskReadyDel(currTsk);
    /* 进队列后默认在队列尾部 */
    OsTskReadyAdd(currTsk);
    if ((g_highestTask != currTsk) && (g_uniTaskLock == 0)) {
        UNI_FLAG |= OS_FLG_TSK_REQ;
    }
    return;
}
#endif

/*
 * 描述: 调度的主入口
 * 备注: NA
 */
OS_SEC_L0_TEXT void OsMainSchedule(void)
{
    struct TagTskCb *prevTsk = RUNNING_TASK;
#if defined(OS_OPTION_RR_SCHED)
    U64 currTime = OsCurCycleGet64();
    OsTimeSliceUpdate(prevTsk, currTime, 0);
#endif
    if ((UNI_FLAG & OS_FLG_TSK_REQ) != 0) {
        /* 清除OS_FLG_TSK_REQ标记位 */
        UNI_FLAG &= ~OS_FLG_TSK_REQ;

        RUNNING_TASK->taskStatus &= ~OS_TSK_RUNNING;
        g_highestTask->taskStatus |= OS_TSK_RUNNING;

        RUNNING_TASK = g_highestTask;

        /* 有任务切换钩子&最高优先级任务等待调度 */
        if (prevTsk != g_highestTask) {
#if defined(OS_OPTION_RR_SCHED)
            g_highestTask->startTime = currTime;
#endif
            OsTskSwitchHookCaller(prevTsk->taskPid, g_highestTask->taskPid);
        }
    }
    // 如果中断没有驱动一个任务ready，直接回到被打断的任务
    OsTskContextLoad((uintptr_t)RUNNING_TASK);
}

/*
 * 描述: 切换任务
 * 备注: 包含任务切换钩子，和任务上下文恢复操作
 */
OS_SEC_L0_TEXT void OsContextSwitch(struct TagTskCb *prev, struct TagTskCb *next)
{
    /* 有任务切换钩子&最高优先级任务等待调度 */
    if (prev != next) {
        OsTskSwitchHookCaller(prev->taskPid, next->taskPid);
    }
    /* 正式切换,prev已经在putprev经过处理，这里跳过 */
    OsTskContextLoad((uintptr_t)next);
}

/*
 * 描述: 系统启动时的首次任务调度
 * 备注: NA
 */
OS_SEC_L4_TEXT void OsFirstTimeSwitch(void)
{
    OsTskHighestSet();
    RUNNING_TASK = g_highestTask;
    TSK_STATUS_SET(RUNNING_TASK, OS_TSK_RUNNING);
#if defined(OS_OPTION_RR_SCHED)
    RUNNING_TASK->startTime = OsCurCycleGet64();
#endif
    OsTskContextLoad((uintptr_t)RUNNING_TASK);
    // never get here
    return;
}

/*
 * 描述: 中断处理流程尾部处理，TICK 、任务调用
 * 备注: NA
 */
OS_SEC_L0_TEXT void OsHwiDispatchTail(void)
{
    U64 irqStartTime = 0;
    if (TICK_NO_RESPOND_CNT > 0) {
        if ((UNI_FLAG & OS_FLG_TICK_ACTIVE) != 0) {
            // OsTskContextLoad， 回到被打断的tick处理现场
            return;
        }
#if defined(OS_OPTION_RR_SCHED) && defined(OS_OPTION_RR_SCHED_IRQ_TIME_DISCOUNT)
        irqStartTime = OsCurCycleGet64();
#endif
        UNI_FLAG |= OS_FLG_TICK_ACTIVE;

        do {
            OsIntEnable();
            // tickISRִ，这里开中断
            g_tickDispatcher();
            OsIntDisable();
            TICK_NO_RESPOND_CNT--;
        } while (TICK_NO_RESPOND_CNT > 0);

        UNI_FLAG &= ~OS_FLG_TICK_ACTIVE;
        OS_IRQ_TIME_RECORD(irqStartTime);
    }
#if defined(OS_OPTION_RR_SCHED)
    OsHwiEndCheckTimeSlice(OsCurCycleGet64());
#endif

    OsMainSchedule();
}
