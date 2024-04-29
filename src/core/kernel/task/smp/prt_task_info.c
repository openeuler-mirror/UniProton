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
 * Description: Task Info implementation
 */
#include "prt_task_internal.h"

#if defined(OS_OPTION_TASK_INFO)
OS_SEC_ALW_INLINE INLINE bool OsTaskSpPcInfoReady(struct TagTskCb *taskCB)
{
    return ((((taskCB->taskStatus & OS_TSK_RUNNING) == 0) || (OS_INT_ACTIVE) || 
        (taskCB->coreID != THIS_CORE())) && OsTaskStackIsSave(taskCB));
}

OS_SEC_L2_TEXT uintptr_t OsTaskSpInfoGet(struct TagTskCb *taskCb)
{
    uintptr_t sp;

    if (OsTaskSpPcInfoReady(taskCb)) {
        sp = (uintptr_t)taskCb->stackPointer;
    } else {
        sp = OsGetSp();
    }

    return sp;
}

OS_SEC_L2_TEXT void OsTaskSpPcGet(struct TagTskCb *taskCb, struct TskInfo *taskInfo)
{
    if (OsTaskSpPcInfoReady(taskCb)) {
        taskInfo->sp = (uintptr_t)taskCb->stackPointer;
        taskInfo->pc = OsTskGetInstrAddr((uintptr_t)taskCb->stackPointer);
    }
    /* 非中断时, 任务若处于运行态, 其控制块的stackPointer值不准确,不能据此获取SP,PC */
    return;
}

OS_SEC_ALW_INLINE INLINE U32 OsTaskInfoGetChk(TskHandle taskPid, struct TskInfo *taskInfo)
{
    if (taskInfo == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }
    return OS_OK;
}

OS_SEC_ALW_INLINE INLINE void OsTaskInfoCommonGet(struct TskInfo *taskInfo, struct TagTskCb *taskCb, TskHandle taskPid)
{
    char *name = NULL;

    taskInfo->taskStatus = (TskStatus)taskCb->taskStatus;
    taskInfo->taskPrio = taskCb->priority;
    taskInfo->stackSize = taskCb->stackSize;
    taskInfo->topOfStack = taskCb->topOfStack;

    if (g_taskNameGet != NULL) {
        g_taskNameGet(taskPid, &name);

        if (strncpy_s(taskInfo->name, sizeof(taskInfo->name), name, (sizeof(taskInfo->name) - 1)) != EOK) {
            OS_GOTO_SYS_ERROR1();
        }
        taskInfo->name[OS_TSK_NAME_LEN - 1] = '\0';
    }

    taskInfo->core = taskCb->coreID;

    taskInfo->entry = taskCb->taskEntry;
    taskInfo->tcbAddr = taskCb;
}
OS_SEC_L4_TEXT void OsTaskStackPeakCal(struct TagTskCb *taskCB,struct TskStackInfo *stackInfo)
{
    (void)taskCB;
    U32 *stack = NULL;
    if (*(U32 *)stackInfo->top == OS_TSK_STACK_TOP_MAGIC) {
        stack = (U32 *)(stackInfo->top + sizeof(U32));
        while ((stack < (U32 *)stackInfo->sp) && (*stack == OS_TSK_STACK_MAGIC)) {
            stack += 1;
        }

        stackInfo->peakUsed = (U32)(stackInfo->bottom - (uintptr_t)stack);
        stackInfo->ovf = FALSE;
    } else {
        stackInfo->peakUsed = OS_MAX_U32;
        stackInfo->ovf = TRUE;
    }
}
OS_SEC_L4_TEXT void OsTaskStackPeakGet(struct TagTskCb *taskCB,struct TskStackInfo *stackInfo)
{
    if (taskCB->stackCfgFlg == OS_TSK_STACK_CFG_BY_SYS) {
        OsTaskStackPeakCal(taskCB, stackInfo);
    } else {
        stackInfo->peakUsed = stackInfo->currUsed;
        stackInfo->ovf = FALSE;
    }
}
/*
 * 描述：获取指定任务的堆栈信息
 */
OS_SEC_L4_TEXT U32 OsTaskStackInfoGet(TskHandle taskPid, struct TskStackInfo *stackInfo)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if (stackInfo == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    taskCb = GET_TCB_HANDLE(taskPid);
    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    /* 获取SP */
    stackInfo->sp = OsTaskSpInfoGet(taskCb);

    stackInfo->top = (uintptr_t)taskCb->topOfStack;
    stackInfo->bottom = TRUNCATE(((uintptr_t)(taskCb->topOfStack) + (taskCb->stackSize)), OS_TSK_STACK_ADDR_ALIGN);
    stackInfo->currUsed = (U32)(stackInfo->bottom - stackInfo->sp);
    OsTaskStackPeakGet(taskCb, stackInfo);
    OsIntRestore(intSave);

    return OS_OK;
}
OS_SEC_ALW_INLINE INLINE U32 OsTaskInfoStackGet(TskHandle taskPid, struct TskInfo *taskInfo)
{
    U32 ret;
    struct TskStackInfo stackInfo = {0};

    ret = OsTaskStackInfoGet(taskPid, &stackInfo);
    if (ret != OS_OK) {
        return ret;
    }

    taskInfo->bottom = stackInfo.bottom;
    taskInfo->currUsed = stackInfo.currUsed;
    taskInfo->peakUsed = stackInfo.peakUsed;
    taskInfo->ovf = stackInfo.ovf;
    return OS_OK;
}

/*
 * 描述：获取指定任务的信息
 */
OS_SEC_L4_TEXT U32 PRT_TaskGetInfo(TskHandle taskPid, struct TskInfo *taskInfo)
{
    U32 ret;
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    ret = OsTaskInfoGetChk(taskPid, taskInfo);
    if (ret != OS_OK) {
        return ret;
    }

    taskCb = GET_TCB_HANDLE(taskPid);
    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    OsTaskSpPcGet(taskCb, taskInfo);

    OsTaskInfoCommonGet(taskInfo, taskCb, taskPid);

    /* 当前在中断中或者不是当前rq的运行任务 */
    if (OS_INT_ACTIVE || (taskPid != RUNNING_TASK->taskPid)) {
        OsTskContextGet((uintptr_t)taskCb->stackPointer, &taskInfo->context);
    }
    OsIntRestore(intSave);

    return OsTaskInfoStackGet(taskPid, taskInfo);
}

/*
 * 描述：获取指定的任务名
 */
OS_SEC_L4_TEXT U32 PRT_TaskGetName(TskHandle taskId, char **name)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if (CHECK_TSK_PID_OVERFLOW(taskId)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    if (name == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    taskCb = GET_TCB_HANDLE(taskId);

    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    if (g_taskNameGet != NULL) {
        g_taskNameGet(taskId, name);
    }

    OsIntRestore(intSave);

    return OS_OK;
}

/*
 * 描述：修改制定的任务名
 */
OS_SEC_L4_TEXT U32 PRT_TaskSetName(TskHandle taskId, const char *name)
{
    uintptr_t intSave;
    struct TagTskCb *taskCb = NULL;

    if (CHECK_TSK_PID_OVERFLOW(taskId)) {
        return OS_ERRNO_TSK_ID_INVALID;
    }

    if (name == NULL) {
        return OS_ERRNO_TSK_PTR_NULL;
    }

    taskCb = GET_TCB_HANDLE(taskId);

    intSave = OsIntLock();

    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_NOT_CREATED;
    }

    if (g_taskNameAdd != NULL) {
        g_taskNameAdd(taskId, name);
    }

    OsIntRestore(intSave);

    return OS_OK;
}
#endif
