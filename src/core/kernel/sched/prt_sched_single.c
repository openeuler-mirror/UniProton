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

/*
 * 描述: 调度的主入口
 * 备注: NA
 */
OS_SEC_L0_TEXT void OsMainSchedule(void)
{
    struct TagTskCb *prevTsk;
    if ((UNI_FLAG & OS_FLG_TSK_REQ) != 0) {
        prevTsk = RUNNING_TASK;

        /* 清除OS_FLG_TSK_REQ标记位 */
        UNI_FLAG &= ~OS_FLG_TSK_REQ;

        RUNNING_TASK->taskStatus &= ~OS_TSK_RUNNING;
        g_highestTask->taskStatus |= OS_TSK_RUNNING;

        RUNNING_TASK = g_highestTask;

        /* 有任务切换钩子&最高优先级任务等待调度 */
        if (prevTsk != g_highestTask) {
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
    if (TICK_NO_RESPOND_CNT > 0) {
        if ((UNI_FLAG & OS_FLG_TICK_ACTIVE) != 0) {
            // OsTskContextLoad， 回到被打断的tick处理现场
            return;
        }
        UNI_FLAG |= OS_FLG_TICK_ACTIVE;

        do {
            OsIntEnable();
            // tickISRִ，这里开中断
            g_tickDispatcher();
            OsIntDisable();
            TICK_NO_RESPOND_CNT--;
        } while (TICK_NO_RESPOND_CNT > 0);

        UNI_FLAG &= ~OS_FLG_TICK_ACTIVE;
    }

    OsMainSchedule();
}
