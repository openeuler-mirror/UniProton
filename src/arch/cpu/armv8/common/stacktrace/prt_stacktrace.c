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
 * Create: 2024-05-13
 * Description: callstack相关的处理。
 */
#include "prt_stacktrace_internal.h"
#include "prt_task_external.h"
#include "prt_task_external.h"

OS_SEC_L4_TEXT U32 OsGetStackTraceParaCheck(U32 *maxNum, uintptr_t *list)
{
    if ((maxNum == NULL) || (list == NULL)) {
        return OS_ERRNO_STACKTRACE_PTR_NULL;
    }

    if ((*maxNum == 0) || (*maxNum > STACKTRACE_MAX_DEPTH)) {
        return OS_ERRNO_STACKTRACE_DEPTH_INVALID;
    }
    return OS_OK;
}

OS_SEC_L2_TEXT U32 PRT_GetStackTrace(U32 *maxNum, uintptr_t *list)
{
    U32 ret;
    ret = OsGetStackTraceParaCheck(maxNum, list);
    if (ret != OS_OK) {
        return ret;
    }

    OsUnwindGetStackTrace(NULL, maxNum, list);
    return OS_OK;
}

/*
* 描述：根据任务ID获取调用栈
*/
OS_SEC_L2_TEXT U32 PRT_GetStackTraceByTaskID(U32 *maxDepth, uintptr_t *list, TskHandle taskPid)
{
    U32 ret;
    bool isSelf = FALSE;
    struct TagTskCb *taskCB = NULL;

    ret = OsGetStackTraceParaCheck(maxDepth, list);
    if (ret != OS_OK) {
        return ret;
    }

    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return OS_ERRNO_STACKTRACE_TSKID_INVALID;
    }

    taskCB = GET_TCB_HANDLE(taskPid);

    if (taskCB->taskStatus == OS_TSK_UNUSED) {
        return OS_ERRNO_STACKTRACE_TSK_UNUSED;
    }

#if defined(OS_OPTION_SMP)
    if (!OsTaskStackIsSave(&taskCB)) {
        return OS_ERRNO_STACKTRACE_CROSS_CORE;
    }
#endif

    // 检查是否就是当前正在运行的任务
#if defined(OS_OPTION_SMP)
    uintptr_t intSave = OsIntLock();
#endif
    bool excFlag = (CUR_NEST_COUNT > 0) ? TRUE : FALSE;
    struct TagTskCb *runningTask = RUNNING_TASK;
    U32 uniFlag = UNI_FLAG;
#if defined(OS_OPTION_SMP)
    OsIntRestore(intSave);
#endif

    // 区分调用场景进行处理
    if (!excFlag) { // 非异常场景
        if ((taskCB == runningTask) && ((uniFlag & OS_FLG_HWI_ACTIVE) == 0)) {
            isSelf = TRUE;
        }
    }

    if (isSelf) {
        OsUnwindGetStackTrace(NULL, maxDepth, list);
        return OS_OK;
    }

    if ((taskCB->taskStatus & OS_TSK_RUNNING) != 0) {
        *maxDepth = 0;
        return OS_ERRNO_STACKTRACE_TSK_RUNNING;
    }

    OsUnwindGetStackTrace(taskCB, maxDepth, list);
    return OS_OK;
}

OS_SEC_L2_TEXT U32 PRT_PrintStackTraceResult(U32 maxNum, uintptr_t *list)
{
    U32 ret = OsGetStackTraceParaCheck(&maxNum, list);
    if (ret != OS_OK) {
        return ret;
    }

    PRT_Printf("The stack trace result is as follows: \n");

    for(U32 i = 0; i < maxNum; i++){
        PRT_Printf("0x%lx, stacktrace: %u\n", list[i], i);
    }

    return OS_OK;
}