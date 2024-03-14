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
 * Description: 事件函数实现
 */
#include "prt_event.h"
#include "prt_task_external.h"
#include "prt_task_sched_external.h"

// 支持功能宏裁剪
#if defined(OS_OPTION_EVENT)

OS_SEC_L4_TEXT U32 OsEventReadParaCheck(U32 eventMask, U32 flags, U32 timeOut)
{
    if (eventMask == 0) {
        return OS_ERRNO_EVENT_EVENTMASK_INVALID;
    }
    /* 读事件模式非法或者（读事件模式为非等待模式且等待时间为0）时，返回错误 */
    if (!(flags == (OS_EVENT_ALL | OS_EVENT_WAIT) || flags == (OS_EVENT_ALL | OS_EVENT_NOWAIT) ||
        flags == (OS_EVENT_ANY | OS_EVENT_WAIT) || flags == (OS_EVENT_ANY | OS_EVENT_NOWAIT)) ||
        (((flags & OS_EVENT_WAIT) != 0) && (timeOut == 0))) {
        return OS_ERRNO_EVENT_FLAGS_INVALID;
    }

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE bool OsIsEventNotMatch(U32 flags, U32 event, U32 eventMask, struct TagTskCb *runTsk)
{
    /* 读事件非期望获得所有事件 */
    if ((flags & OS_EVENT_ALL) != 0) {
        TSK_STATUS_SET(runTsk, OS_TSK_EVENT_TYPE);
        if ((eventMask != (event & eventMask))) {
            return TRUE;
        }
    } else {
        TSK_STATUS_CLEAR(runTsk, OS_TSK_EVENT_TYPE);
        if ((event & eventMask) == 0) {
            return TRUE;
        }
    }

    return FALSE;
}

OS_SEC_ALW_INLINE INLINE void OsEventTimeOutSet(U32 timeOut, struct TagTskCb *runTsk)
{
    if (timeOut == OS_EVENT_WAIT_FOREVER) {
        TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);
    } else {
        TSK_STATUS_SET(runTsk, OS_TSK_TIMEOUT);
        OsTskTimerAdd(runTsk, timeOut);
    }
}

OS_SEC_ALW_INLINE INLINE U32 OsEventReadNeedSche(struct TagOsRunQue **runQue, U32 flags, struct TagTskCb *runTsk,
                                                 U32 timeOut, U32 *event)
{
    /* 读事件处于等待读取模式 */
    if ((flags & OS_EVENT_NOWAIT) != 0) {
        return OS_ERRNO_EVENT_READ_FAILED;
    }

    /* 如果锁任务的情况下 */
    if (OS_TASK_LOCK_DATA != 0) {
        return OS_ERRNO_EVENT_READ_IN_LOCK;
    }

    OsTskReadyDel(runTsk);

    TSK_STATUS_SET(runTsk, OS_TSK_EVENT_PEND);

    OsEventTimeOutSet(timeOut, runTsk);
    
    OsSpinUnLockRunTaskRq(*runQue);

    OsTskSchedule();

    *runQue = OsSpinLockRunTaskRq();

    /* 判断是否超时返回并做超时处理 */
    if ((runTsk->taskStatus & OS_TSK_TIMEOUT) != 0) {
        TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);
        return OS_ERRNO_EVENT_READ_TIMEOUT;
    }

    *event = runTsk->event;
    return OS_OK;
}

/*
 * 描述：读事件操作
 */
OS_SEC_L4_TEXT U32 PRT_EventRead(U32 eventMask, U32 flags, U32 timeOut, U32 *events)
{
    U32 ret;
    U32 event;
    uintptr_t intSave;
    struct TagTskCb *runTsk = NULL;
    bool needSchedule = FALSE;
    struct TagOsRunQue *runQue = NULL;

    ret = OsEventReadParaCheck(eventMask, flags, timeOut);
    if (ret != OS_OK) {
        return ret;
    }

    intSave = OsIntLock();

    if (OS_INT_ACTIVE) {
        OsIntRestore(intSave);
        return OS_ERRNO_EVENT_READ_NOT_IN_TASK;
    }

    runTsk = RUNNING_TASK;
    runQue = OsSpinLockRunTaskRq();
    runTsk->eventMask = eventMask;
    event = runTsk->event;

    needSchedule = OsIsEventNotMatch(flags, event, eventMask, runTsk);
    /* 期望的事件一件都没有发生,或者在OS_EVENT_ALL情况下没有发生期望所有事件 */
    if (needSchedule == TRUE) {
        ret = OsEventReadNeedSche(&runQue, flags, runTsk, timeOut, &event);
        if (ret != OS_OK) {
            OsSpinUnLockRunTaskRq(runQue);
            OsIntRestore(intSave);
            return ret;
        }
    }

    /* 清除已经读取到的事件 */
    runTsk->event &= (~eventMask);

    if (events != NULL) {
        *events = event & eventMask;
    }
    OsSpinUnLockRunTaskRq(runQue);
    OsIntRestore(intSave);

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsEventStateChange(U32 taskStatus, struct TagTskCb *taskCb)
{
    if ((taskStatus & OS_TSK_TIMEOUT) != 0) {
        /*
         * 添加PEND状态时，会加上TIMEOUT标志和timer，或者都不加。
         * 所以此时有TIMEOUT标志就一定有timer，且只有一个，且只用于该PEND方式
         */
        OS_TSK_DELAY_LOCKED_DETACH(taskCb);
        TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT);
    }

    if ((OS_TSK_SUSPEND & taskStatus) == 0) {
        OsTskReadyAddBgd(taskCb);
    }
}

/*
 * 描述：写事件操作
 */
OS_SEC_L4_TEXT U32 PRT_EventWrite(U32 taskId, U32 events)
{
    struct TagTskCb *taskCb = NULL;
    uintptr_t intSave;
    bool needSchedule = FALSE;
    U32 eventMask;
    U32 taskStatus;

    if (CHECK_TSK_PID_OVERFLOW(taskId)) {
        return OS_ERRNO_EVENT_TASKID_INVALID;
    }

    if (events == 0) {
        return OS_ERRNO_EVENT_INVALID;
    }

    intSave = OsIntLock();
    taskCb = (struct TagTskCb *)GET_TCB_HANDLE(taskId);
    OsSpinLockTaskRq(taskCb);

    taskStatus = taskCb->taskStatus;
    if (TSK_IS_UNUSED(taskCb)) {
        OsSpinUnlockTaskRq(taskCb);
        OsIntRestore(intSave);

        return OS_ERRNO_EVENT_TSK_NOT_CREATED;
    }

    taskCb->event |= events;

    /* 判断目的线程是否阻塞于读事件 */
    if ((taskStatus & OS_TSK_EVENT_PEND) != 0) {
        eventMask = taskCb->eventMask;

        /* 判断是否需要任务调度 */
        if ((taskStatus & OS_TSK_EVENT_TYPE) != 0) {
            if (eventMask == (taskCb->event & eventMask)) {
                needSchedule = TRUE;
            }
        } else {
            if ((taskCb->event & eventMask) != 0) {
                needSchedule = TRUE;
            }
        }

        if (needSchedule) {
            TSK_STATUS_CLEAR(taskCb, OS_TSK_EVENT_PEND);

            OsEventStateChange(taskStatus, taskCb);

            OsSpinUnlockTaskRq(taskCb);

            OsTskSchedule();

            OsIntRestore(intSave);
            return OS_OK;
        }
    }
    OsSpinUnlockTaskRq(taskCb);
    OsIntRestore(intSave);
    return OS_OK;
}

#endif
