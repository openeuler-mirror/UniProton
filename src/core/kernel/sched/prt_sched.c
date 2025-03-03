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
 * Description: 调度的公共函数实现
 */
#include "prt_sched_external.h"
#include "prt_hook_external.h"
#include "prt_task_external.h"
#include "prt_rt_external.h"
#include "prt_perf.h"

OS_SEC_BSS struct TagOsRunQue g_runQueue[OS_MAX_CORE_NUM]; // 核的局部运行队列

/*
 * 描述：发送核间中断出发目标核调度不需要,锁由外部中断保证
 * 备注：后续不支持插拔核，前向收编统一低功功耗方案
 */
OS_SEC_L0_TEXT void OsSmpSendReschedule(U32 coreID)
{
    struct TagOsRunQue *runQue = GET_RUNQ(coreID);

    if (!runQue->needReschedule) {
        return;
    }

    OsCoreWakeupIpc(coreID);
    SMP_MC_SCHEDULE_TRIGGER(coreID);
    return;
}

static OS_SEC_L0_TEXT void OsSmpSendRescheduleNoWake(U32 coreID)
{
    struct TagOsRunQue *runQue = GET_RUNQ(coreID);

    if (!runQue->needReschedule) {
        return;
    }

    SMP_MC_SCHEDULE_TRIGGER(coreID);
    return;
}

/*
 * 描述：触发调度中断
 * 备注：本核则仅置位
 */
OS_SEC_L0_TEXT void OsReschedTask(struct TagTskCb *task)
{
    U32 coreId;

    coreId = task->coreID;
    if(LIKELY(coreId == THIS_CORE())) {
        return;
    }
    OsSmpSendReschedule(coreId);
}

OS_SEC_L0_TEXT void OsReschedTaskNoWakeIpc(struct TagTskCb *task)
{
    U32 coreId;

    coreId = task->coreID;
    if(LIKELY(coreId == THIS_CORE())) {
        return;
    }
    OsSmpSendRescheduleNoWake(task->coreID);
    return;
}

/*
 * 描述：任务尝试切出，外部保证关中断
 * 备注：中断仅置位，留到中断推出侯调度
 */
OS_SEC_L0_TEXT void OsTskSchedule(void)
{
#if defined(OS_OPTION_PERF) && defined(OS_OPTION_PERF_SW_PMU)
    PRT_PERF(TASK_SWITCH);
#endif
    struct TagOsRunQue *runQue = THIS_RUNQ();
    
    if ((runQue->uniFlag & OS_INT_ACTIVE_MASK) != 0 ) {
        return;
    }

    if (!runQue->needReschedule) {
        return;
    }
    OsTaskTrap((uintptr_t)runQue->tskCurr);
    return;
}

OS_SEC_L2_TEXT bool OsTskSchedNextRtTask(U32 core, TskHandle *tid, U32 *coreId)
{
    bool needSchedule = FALSE;
    struct TagOsRunQue *rq = NULL;
    struct TagTskCb *tsk = NULL;

    rq = GET_RUNQ(core);
    OsSplLock(&rq->spinLock);
    if(rq->needReschedule == TRUE) {
        needSchedule = TRUE;
        tsk = rq->schedClass->osNextReadyTask(rq);
        *tid = tsk->taskPid;
        *coreId = core;
    }
    OsSplUnlock(&rq->spinLock);
    return needSchedule;
}

/*
 * 描述：切换任务
 * 备注：包含任务切换钩子和其它必要的任务切换附带操作。
 */
OS_SEC_L0_TEXT void OsContextSwitch(struct TagTskCb *prev, struct TagTskCb *next)
{
    OsTskSwitchHookCaller(prev->taskPid, next->taskPid);

    OsTskContextLoad((uintptr_t)next);
}

#if defined(OS_OPTION_RR_SCHED)
/* 更新当前任务的剩余时间片，并检查是否需要切换 */
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
    return;
}
#endif

/*
 * 描述：调度的主入口
 * 备注：NA
 */
OS_SEC_L0_TEXT void OsMainSchedule(void)
{
    struct TagTskCb *taskOrigin = NULL;
    struct TagTskCb *taskNext = NULL;

    // U32 thisCore = OsGetHwThreadId();
    struct TagOsRunQue *runQue = THIS_RUNQ();

    taskOrigin = runQue->tskCurr;
#if defined(OS_OPTION_RR_SCHED)
    U64 currTime = OsCurCycleGet64();
    OsTimeSliceUpdate(taskOrigin, currTime, 0);
#endif
    if (runQue->uniTaskLock == 0) {
        OsWorkHandler();

        if (runQue->needReschedule != 0) {
            OS_SMP_HANDSHAKE_COUNT(THIS_CORE())++;
            do {
                taskNext = runQue->schedClass->osPickNextTask(runQue);
            } while (runQue->needReschedule);

            if(taskNext != taskOrigin) {
#if defined(OS_OPTION_RR_SCHED)
                taskNext->startTime = currTime;
#endif
                OsContextSwitch(taskOrigin, taskNext);
            }
        }
    }

    OsTskContextLoad((uintptr_t)taskOrigin);
}