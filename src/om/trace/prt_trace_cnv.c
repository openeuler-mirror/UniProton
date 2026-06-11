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
 * Create: 2024-06-10
 * Description: Trace钩子转换，注册Trace事件到内核各模块的Hook。
 */
#include "prt_trace_internal.h"
#include "prt_task_external.h"
#include "prt_hwi_external.h"
#include "prt_hook_external.h"

#ifdef OS_OPTION_TRACE

static void OsTraceTaskSwitchedIn(uintptr_t prevPid, uintptr_t nextPid)
{
    struct TagTskCb *prevTask = GET_TCB_HANDLE((U32)prevPid);
    struct TagTskCb *nextTask = GET_TCB_HANDLE((U32)nextPid);
    PRT_TRACE(TASK_SWITCH, nextTask->taskPid, prevTask->priority,
        prevTask->taskStatus, nextTask->priority, nextTask->taskStatus);
}

static void OsTraceTaskCreate(uintptr_t taskCBAddr)
{
    struct TagTskCb *taskCB = (struct TagTskCb *)taskCBAddr;
    PRT_TRACE(TASK_CREATE, taskCB->taskPid, taskCB->taskStatus, (uintptr_t)taskCB->priority);
}

static void OsTraceTaskDelete(uintptr_t taskCBAddr)
{
    struct TagTskCb *taskCB = (struct TagTskCb *)taskCBAddr;
    PRT_TRACE(TASK_DELETE, taskCB->taskPid, taskCB->taskStatus, (uintptr_t)taskCB->stackPointer);
}

static void OsTraceIsrEnter(uintptr_t hwiNum)
{
    PRT_TRACE(HWI_RESPONSE_IN, hwiNum);
}

static void OsTraceIsrExit(uintptr_t hwiNum)
{
    PRT_TRACE(HWI_RESPONSE_OUT, hwiNum);
}

void OsTraceCnvInit(void)
{
    OsHookAdd(OS_HOOK_TSK_SWITCH, (OsVoidFunc)OsTraceTaskSwitchedIn);
    OsHookAdd(OS_HOOK_TSK_CREATE, (OsVoidFunc)OsTraceTaskCreate);
    OsHookAdd(OS_HOOK_TSK_DELETE, (OsVoidFunc)OsTraceTaskDelete);
    OsHookAdd(OS_HOOK_HWI_ENTRY, (OsVoidFunc)OsTraceIsrEnter);
    OsHookAdd(OS_HOOK_HWI_EXIT, (OsVoidFunc)OsTraceIsrExit);
}

#endif /* OS_OPTION_TRACE */