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
 * Create: 2024-01-25
 * Description: 调度类
 */
#include "prt_sched_external.h"
#include "prt_task_external.h"
/*
 * 描述：从目标运行队列上移出任务
 * 备注：NA
 */
OS_SEC_L0_TEXT void OsDequeueTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 sleep)
{
    tsk->scheClass->osDequeueTask(runQue, tsk, sleep);
    tsk->isOnRq = FALSE;
}

/*
 * 描述：在目标队列上添加任务
 * 备注：NA
 */
OS_SEC_L0_TEXT void OsEnqueueTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags)
{
    tsk->scheClass->osEnqueueTask(runQue, tsk, flags);
    tsk->isOnRq = TRUE;
}

/*
 * 描述：从目标队列上去激活任务
 * 备注：NA
 */
OS_SEC_L0_TEXT void OsDeactiveTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags)
{
    if (!tsk->isOnRq) {
        return;
    }
    OsDequeueTask(runQue, tsk, flags);

    OsDecNrRunning(runQue);
}

/*
 * 描述：将任务入队激活
 * 备注：NA
 */
OS_SEC_L0_TEXT void OsActiveTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags)
{
    if (tsk->isOnRq) {
        return;
    }
    OsEnqueueTask(runQue, tsk, flags);

    OsIncNrRunning(runQue);
}

/*
 * 描述：获取下一个运行任务，待切入任务，调度的pick主函数
 * 备注：NA
 */
OS_SEC_L0_TEXT struct TagTskCb *OsPickNextTask(struct TagOsRunQue *runQue)
{
    return runQue->schedClass->osPickNextTask(runQue);
}