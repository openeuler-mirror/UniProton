/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-08
 * Description: 信号模块
 */
#include "signal.h"
#include "prt_signal.h"
#include "prt_signal_external.h"
#include "prt_hook_external.h"
#include "prt_task_external.h"

OS_SEC_L4_TEXT void OsSigDefaultHandler(int signum)
{
    /* do nothing */
    return;
}

OS_SEC_L4_TEXT void OsInitSigVectors(struct TagTskCb *taskCb)
{
    for (int i = 0; i < PRT_SIGNAL_MAX; i++) {
        taskCb->sigVectors[i] = OsSigDefaultHandler;
    }
    return;
}

OS_SEC_ALW_INLINE INLINE void OsSignalTimeOutSet(struct TagTskCb *runTsk, U32 timeOutTick)
{
    if (timeOutTick == OS_SIGNAL_WAIT_FOREVER) {
        TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);
    } else {
        TSK_STATUS_SET(runTsk, OS_TSK_TIMEOUT);
        OsTskTimerAdd(runTsk, timeOutTick);
    }
}

OS_SEC_ALW_INLINE INLINE U32 OsSignalWaitSche(struct TagTskCb *runTsk, const signalSet *set, U32 timeOutTick)
{
    if (timeOutTick == 0) {
        return OS_ERRNO_SIGNAL_PARA_INVALID;
    }

    if (OS_TASK_LOCK_DATA != 0) {
        return OS_ERRNO_SIGNAL_WAIT_IN_LOCK;
    }

    runTsk->sigWaitMask = *set;

    OsTskReadyDel(runTsk);

    TSK_STATUS_SET(runTsk, OS_TSK_WAIT_SIGNAL);

    OsSignalTimeOutSet(runTsk, timeOutTick);

    OsTskSchedule();

    /* 任务返回上下文 */
    if ((runTsk->taskStatus & OS_TSK_TIMEOUT) != 0) {
        TSK_STATUS_CLEAR(runTsk, OS_TSK_TIMEOUT);
        return OS_ERRNO_SIGNAL_TIMEOUT;
    }

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsSigWaitProcPendingSignal(struct TagTskCb *runTsk, const signalSet *set, signalInfo *info)
{
    sigInfoNode *infoNode = NULL;
    LIST_FOR_EACH(infoNode, &runTsk->sigInfoList, sigInfoNode, siglist) {
        if ((sigMask(infoNode->siginfo.si_signo) & *set) == 0) {
            continue;
        }

        (void)memcpy_s(info, sizeof(signalInfo), &(infoNode->siginfo), sizeof(signalInfo));

        /* 清除pending标记 */
        runTsk->sigPending &= ~sigMask(infoNode->siginfo.si_signo);
        ListDelete(&(infoNode->siglist));
        PRT_MemFree((U32)OS_MID_SIGNAL, infoNode);
        break;
    }

    /* 已等到信号，清除等待信号集 */
    runTsk->sigWaitMask = 0;

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE U32 OsAddSignalPendingFlag(struct TagTskCb *taskCb, signalInfo *info)
{
    sigInfoNode *infoNode = NULL;
    /* 若未决信号已存在，则更新未决信号对应的数据 */
    if ((taskCb->sigPending & sigMask(info->si_signo)) != 0) {
        LIST_FOR_EACH(infoNode, &taskCb->sigInfoList, sigInfoNode, siglist) {
            if (infoNode->siginfo.si_signo != info->si_signo) {
                continue;
            }

            (void)memcpy_s(&infoNode->siginfo, sizeof(signalInfo), info, sizeof(signalInfo));
            break;
        }
        return OS_OK;
    }

    infoNode = (sigInfoNode *)PRT_MemAlloc((U32)OS_MID_SIGNAL, OS_MEM_DEFAULT_FSC_PT, sizeof(sigInfoNode));
    if (infoNode == NULL) {
        return OS_ERRNO_SIGNAL_NO_MEMORY;
    }
    
    (void)memcpy_s(&infoNode->siginfo, sizeof(signalInfo), info, sizeof(signalInfo));
    ListTailAdd(&infoNode->siglist, &taskCb->sigInfoList);
    taskCb->sigPending |= sigMask(info->si_signo);

    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsSignalWaitReSche(struct TagTskCb *taskCb, U32 taskStatus)
{
    if ((taskStatus & OS_TSK_TIMEOUT) != 0) {
        OS_TSK_DELAY_LOCKED_DETACH(taskCb);
        TSK_STATUS_CLEAR(taskCb, OS_TSK_TIMEOUT);
    }

    if ((OS_TSK_SUSPEND & taskStatus) == 0) {
        OsTskReadyAddBgd(taskCb);
    }

    OsTskSchedule();
    return;
}

OS_SEC_ALW_INLINE INLINE void OsHandleOneSignal(struct TagTskCb *runTsk, int signum)
{
    sigInfoNode *infoNode = NULL;
    LIST_FOR_EACH(infoNode, &runTsk->sigInfoList, sigInfoNode, siglist) {
        if (infoNode->siginfo.si_signo != signum) {
            continue;
        }

        _sa_handler handler = runTsk->sigVectors[signum];
        if (handler != NULL) {
            handler(signum);
        }

        /* 清除pending标记 */
        runTsk->sigPending &= ~sigMask(signum);
        ListDelete(&(infoNode->siglist));
        PRT_MemFree((U32)OS_MID_SIGNAL, infoNode);
        break;
    }
    return;
}

OS_SEC_L4_TEXT void OsHandleUnBlockSignal(struct TagTskCb *runTsk)
{
    signalSet unBlockSignal = (runTsk->sigPending & ~(runTsk->sigMask));
    if (unBlockSignal == 0) {
        return;
    }

    sigInfoNode *infoNode = NULL;
    sigInfoNode *tmpNode = NULL;
    LIST_FOR_EACH(infoNode, &runTsk->sigInfoList, sigInfoNode, siglist) {
        int signum = infoNode->siginfo.si_signo;
        if ((unBlockSignal & sigMask(signum)) == 0) {
            continue;
        }

        _sa_handler handler = runTsk->sigVectors[signum];
        if (handler != NULL) {
            handler(signum);
        }

        /* 清除pending标记 */
        runTsk->sigPending &= ~sigMask(signum);
        tmpNode = LIST_COMPONENT(infoNode->siglist.prev, sigInfoNode, siglist);
        ListDelete(&(infoNode->siglist));
        PRT_MemFree((U32)OS_MID_SIGNAL, infoNode);
        infoNode = tmpNode;

        unBlockSignal = (runTsk->sigPending & ~(runTsk->sigMask));
        if (unBlockSignal == 0) {
            break;
        }
    }

    return;
}

OS_SEC_L4_TEXT void OsSignalEntry(TskHandle taskId)
{
    struct TagTskCb *runTsk = RUNNING_TASK;
    int signum = runTsk->holdSignal;
    OsHandleOneSignal(runTsk, signum);

    /* 恢复任务在处理信号之前的上下文，继续执行 */
    runTsk->taskStatus &= ~OS_TSK_HOLD_SIGNAL;
    runTsk->stackPointer = runTsk->oldStackPointer;
    runTsk->holdSignal = 0;
    runTsk->oldStackPointer = NULL;
    OsTskContextLoad((uintptr_t)runTsk);

    return;
}

OS_SEC_L4_TEXT U32 PRT_SignalDeliver(TskHandle taskId, signalInfo *info)
{
    if (CHECK_TSK_PID_OVERFLOW(taskId) || info == NULL) {
        return OS_ERRNO_SIGNAL_PARA_INVALID;
    }

    int signum = info->si_signo;
    if (!sigValid(signum)) {
        return OS_ERRNO_SIGNAL_NUM_INVALID;
    }

    uintptr_t intSave = OsIntLock();
    struct TagTskCb *taskCb = (struct TagTskCb *)GET_TCB_HANDLE(taskId);
    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_SIGNAL_TSK_NOT_CREATED;
    }

    /* 将信号加入pending中，若已存在则更新，不存在则创建 */
    U32 ret = OsAddSignalPendingFlag(taskCb, info);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    /* 任务是否在等待该信号，若是则将该任务加入ready队列，触发一次调度 */
    if (((taskCb->taskStatus & OS_TSK_WAIT_SIGNAL) != 0) && ((taskCb->sigWaitMask & sigMask(signum)) != 0)) {
        TSK_STATUS_CLEAR(taskCb, OS_TSK_WAIT_SIGNAL);
        OsSignalWaitReSche(taskCb, taskCb->taskStatus);
        OsIntRestore(intSave);
        return OS_OK;
    }

    /* 信号是否为阻塞信号，若是则不返回不处理 */
    if ((taskCb->sigMask & sigMask(signum)) != 0) {
        OsIntRestore(intSave);
        return OS_OK;
    }

    /* 判断信号是否是当前的任务处理，若是则处理 */
    if (taskId == RUNNING_TASK->taskPid) {
        OsHandleOneSignal(taskCb, signum);
        OsIntRestore(intSave);
        return OS_OK;
    }

    /* 若任务在就绪队列或者被延时且没有在等待调度去处理信号，则将该任务的上下文保存并使下次调度直接进入信号处理函数*/
    if ((taskCb->taskStatus & (OS_TSK_READY | OS_TSK_DELAY | OS_TSK_SIG_PAUSE | OS_TSK_DELAY_INTERRUPTIBLE)) &&
        !(taskCb->taskStatus & OS_TSK_HOLD_SIGNAL)) {
        taskCb->taskStatus |= OS_TSK_HOLD_SIGNAL;

        taskCb->oldStackPointer = taskCb->stackPointer;
        taskCb->holdSignal = signum;
        U32 curStackSize = (U32)taskCb->stackPointer - (U32)taskCb->topOfStack;
        taskCb->stackPointer = OsTskContextInit(taskId, curStackSize, (uintptr_t *)taskCb->topOfStack,
            (uintptr_t)OsSignalEntry);
        /* 如果在暂停状态，收到信号后任务加回就绪队列*/
        if (taskCb->taskStatus & OS_TSK_SIG_PAUSE) {
            taskCb->taskStatus &= ~OS_TSK_SIG_PAUSE;
            OsTskReadyAdd(taskCb);
        }
#if defined(OS_OPTION_LINUX)
        /* 任务如果在TASK_INTERRUPTIBLE状态收到信号会唤醒*/
        else if (KTHREAD_TSK_STATE_TST(taskCb, TASK_INTERRUPTIBLE)) {
            wake_up_process(taskCb->kthreadTsk);
        }
#endif
        OsTskSchedule();
    }

    OsIntRestore(intSave);
    return OS_OK;
}

OS_SEC_L4_TEXT U32 PRT_SignalWait(const signalSet *set, signalInfo *info, U32 timeOutTick)
{
    U32 ret = OS_OK;

    if (set == NULL || *set == 0 || info == NULL) {
        return OS_ERRNO_SIGNAL_PARA_INVALID;
    }

    if (memset_s(info, sizeof(signalInfo), 0, sizeof(signalInfo)) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    uintptr_t intSave = OsIntLock();
    if (OS_INT_ACTIVE) {
        OsIntRestore(intSave);
        return OS_ERRNO_SIGNAL_WAIT_NOT_IN_TASK;
    }

    struct TagTskCb *runTsk = RUNNING_TASK;
    /* 等待信号集中已有未决信号 */
    if ((runTsk->sigPending & *set) != 0) {
        ret = OsSigWaitProcPendingSignal(runTsk, set, info);
        OsIntRestore(intSave);
        return ret;
    }

    /* 等待信号集中没有未决信号则需要将当前任务挂起等待 */
    ret = OsSignalWaitSche(runTsk, set, timeOutTick);
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        return ret;
    }

    ret = OsSigWaitProcPendingSignal(runTsk, set, info);
    OsIntRestore(intSave);
    return ret;
}

OS_SEC_L4_TEXT U32 PRT_SignalMask(int how, const signalSet *set, signalSet *oldset)
{
    uintptr_t intSave = OsIntLock();
    struct TagTskCb *runTsk = RUNNING_TASK;
    if (oldset != NULL) {
        *oldset = runTsk->sigMask;
    }

    if (set != NULL) {
        switch(how) {
            case SIG_BLOCK:
                runTsk->sigMask |= *set;
                break;
            case SIG_UNBLOCK:
                runTsk->sigMask &= ~*set;
                OsHandleUnBlockSignal(runTsk);
                break;
            case SIG_SETMASK:
                runTsk->sigMask = *set;
                OsHandleUnBlockSignal(runTsk);
                break;
            default:
                break;
        }
    }

    OsIntRestore(intSave);
    return OS_OK;
}

OS_SEC_L4_TEXT U32 PRT_SignalAction(int signum, const struct _sigaction *act, struct _sigaction *oldact)
{
    if (!sigValid(signum)) {
        return OS_ERRNO_SIGNAL_NUM_INVALID;
    }

    uintptr_t intSave = OsIntLock();
    struct TagTskCb *runTsk = RUNNING_TASK;

    if (oldact != NULL) {
        oldact->saHandler = runTsk->sigVectors[signum];
    }

    if (act == NULL) {
        OsIntRestore(intSave);
        return OS_OK;
    }

    _sa_handler handler = act->saHandler;
    if (handler == SIG_IGN) {
        runTsk->sigVectors[signum] = NULL;
    } else if (handler == SIG_DFL) {
        runTsk->sigVectors[signum] = OsSigDefaultHandler;
    } else {
        runTsk->sigVectors[signum] = handler;
    }

    OsIntRestore(intSave);
    return OS_OK;
}

OS_SEC_L4_TEXT U32 PRT_SigSuspend(const signalSet *mask)
{
    U32 ret = OS_OK;
    uintptr_t intSave = OsIntLock();

    struct TagTskCb *runTsk = RUNNING_TASK;

    signalSet saveedMask = runTsk->sigMask;
    runTsk->sigMask = *mask;
    runTsk->sigWaitMask = 0;

    signalSet waitMask = ~(*mask);

    /* 等待信号集中已有未屏蔽信号, 则处理未屏蔽信号，返回 */
    if ((runTsk->sigPending & waitMask) != 0) {
        OsHandleUnBlockSignal(runTsk);
        runTsk->sigMask = saveedMask;
        OsIntRestore(intSave);
    } else {
        if (runTsk->taskPid == IDLE_TASK_ID) {
            return OS_ERRNO_SIGNAL_TASKID_INVALID;
        }
        OsIntRestore(intSave);
        /* 等待信号集中没有未屏蔽信号则需要将当前任务挂起等待 */
        ret = OsSignalWaitSche(runTsk, &waitMask, OS_SIGNAL_WAIT_FOREVER);
        if (ret != OS_OK) {
            return ret;
        }

        OsHandleUnBlockSignal(runTsk);
        runTsk->sigMask = saveedMask;
    }

    return ret;
}
