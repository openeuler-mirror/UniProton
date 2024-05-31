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
 * Description: callstack模块对外头文件。
 */
#ifndef PRT_STACKTRACE_H
#define PRT_STACKTRACE_H

#include "prt_module.h"
#include "prt_errno.h"
#include "prt_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

/*
 * 调用栈相关信息错误码：获取函数调用栈传入的参数为NULL
 *
 * 值: 0x02003501
 *
 * 解决方案: 请确保传入的参数不为NULL。
 */
#define OS_ERRNO_STACKTRACE_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_STACKTRACE, 0x01)

/*
 * 调用栈相关信息错误码：获取函数调用栈传入的最大调用层数非法
 *
 * 值: 0x02003502
 *
 * 解决方案: 请确保传入的最大调用层数大于0且小于10
 */
#define OS_ERRNO_STACKTRACE_DEPTH_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_STACKTRACE, 0x02)

/*
 * 调用栈相关信息错误码：传入的任务ID非法
 *
 * 值: 0x02003504
 *
 * 解决方案: 请确保传入的任务ID合法
 */
#define OS_ERRNO_STACKTRACE_TSKID_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_STACKTRACE, 0x03)

/*
 * 调用栈相关信息错误码：传入的任务ID未创建
 *
 * 值: 0x02003505
 *
 * 解决方案: 请确保传入的任务ID是已经创建了的任务
 */
#define OS_ERRNO_STACKTRACE_TSK_UNUSED OS_ERRNO_BUILD_ERROR(OS_MID_STACKTRACE, 0x04)

/*
 * 调用栈相关信息错误码：传入的任务ID正在运行
 *
 * 值: 0x02003508
 *
 * 解决方案: 确保任务不在running状态
 */
#define OS_ERRNO_STACKTRACE_TSK_RUNNING OS_ERRNO_BUILD_ERROR(OS_MID_STACKTRACE, 0x05)

/*
 * 调用栈相关信息错误码：不支持跨核对由用户配置栈空间的任务进行调用栈推导
 *
 * 值: 0x02003509
 *
 * 解决方案: 在目标任务的运行核上进行调用栈推导
 */
#define OS_ERRNO_STACKTRACE_CROSS_CORE OS_ERRNO_BUILD_ERROR(OS_MID_STACKTRACE, 0x06)

/* 调用栈解析的最大层数 */
#define STACKTRACE_MAX_DEPTH 10

extern U32 PRT_GetStackTrace(U32 *maxNum, uintptr_t *list);

extern U32 PRT_GetStackTraceByTaskID(U32 *maxDepth, uintptr_t *list, TskHandle taskPid);

extern U32 PRT_PrintStackTraceResult(U32 maxNum, uintptr_t *list);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* PRT_SEM_H */
