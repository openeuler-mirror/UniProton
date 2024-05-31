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
 * Description: callstack相关的内部公共头文件
 */
#ifndef PRT_STACKTRACE_EXTERNAL_H
#define PRT_STACKTRACE_EXTERNAL_H

#include "prt_stacktrace.h"
#include "prt_task_external.h"
#include "prt_irq_external.h"
#include "prt_exc_external.h"
#include "prt_sys_external.h"

#if defined(OS_OPTION_TASK)
/*
* 描述：异常时获取发生异常的PID
*/
OS_SEC_ALW_INLINE INLINE U32 OsStackTraceGetCurPid(void)
{
    if (CUR_NEST_COUNT > 0) {
        /* 中断时发生异常 */
        if (OS_INT_COUNT > CUR_NEST_COUNT) {
            return OS_SYS_CONTEXT_PID;
        }
        /* 使用系统栈的场景，注意，异常的时候会将OS_FLG_HWI_ACTIVE和OS_FLG_EXC_ACTIVE置起来 */
        if ((UNI_FLAG & (OS_INT_ACTIVE_MASK & ~(OS_FLG_HWI_ACTIVE | OS_FLG_EXC_ACTIVE))) != 0) {
            return OS_SYS_CONTEXT_PID;
        }
        /* 任务已开始调度 */
        if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) != 0) {
            return RUNNING_TASK->taskPid;
        } else {
            // 任务模块还没开始调度，RUNNING_TASK为NULL
            return OS_SYS_CONTEXT_PID;
        }
    } else {
        return OsGetCurPid();
    }
}
#else
OS_SEC_ALW_INLINE INLINE U32 OsStackTraceGetCurPid(void)
{
    return OS_SYS_CONTEXT_PID;
}
#endif

#endif /* PRT_STACKTRACE_EXTERNAL_H */
