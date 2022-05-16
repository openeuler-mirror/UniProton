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
 * Description: cpup模块的模块内头文件
 */
#ifndef PRT_CPUP_THREAD_INTERNAL_H
#define PRT_CPUP_THREAD_INTERNAL_H

#include "prt_lib_external.h"
#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_cpup_external.h"
#include "prt_mem_external.h"
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "prt_hook_external.h"
#include "prt_exc_external.h"
#include "prt_tick_external.h"
#include "prt_cpup_internal.h"

/*
 * 模块内宏定义
 */
#define OS_CPUP_ENTRY_FLAG 0 /* 处理中断进入钩子时需要计算CPUP标志 */
#define OS_CPUP_EXIT_FLAG 1 /* 处理中断退出钩子时需要计算CPUP标志 */

#define OS_CPUP_LOW_VALUE 0xffffffff

#define OS_CPUP_PTR(taskId) (&g_cpup[TSK_GET_INDEX((taskId))])

#define OS_TASK_CYCLE_START(taskId, curCycle) (OS_CPUP_PTR(taskId)->startTime = (curCycle))

#define OS_TASK_CYCLE_END(taskId, curCycle) \
    (OS_CPUP_PTR(taskId)->allTime += ((curCycle) - OS_CPUP_PTR(taskId)->startTime))

/* 硬中断、Tick钩子是否需要计算CPUP标识 */
extern U32 g_cpupFlag;
#define CPUP_FLAG g_cpupFlag

extern U16 g_cpupDelTask;
extern U64 g_cpuWinStart;
extern U64 g_cpuTimeDelTask;

/*
 * 模块内函数声明
 */
extern U32 OsCpupThreadNow(void);
extern void OsCpupThreadTickTask(void);
extern void OsCpupFirstSwitch(void);
extern void OsCpupTskSwitch(U32 lastTaskId, U32 nextTaskId);
extern void OsNowTskCycleStart(void);
extern void OsNowTskCycleEnd(void);
extern U32 OsCpupPreCheck(void);
extern U16 OsCpupIntGet(void);
extern U64 OsCpupAllTaskTimeGet(void);
extern void OsCpupStartEnd(U32 lastTaskId, U32 nextTaskId, U64 curCycle);
extern void OsCpupTickCal(void);
extern void OsCpupTimeClear(void);

OS_SEC_ALW_INLINE INLINE U64 OsCpupGetWinCycles(U64 curCycle)
{
    U64 cycles = curCycle - g_cpuWinStart;

    /* 理论上当前cycle必然大于时间窗起始值，如果相等，即认为当前cycle值已翻转，取时间窗大小为最大值 */
    if (cycles == 0) {
        cycles = (U64)(-1);
    }

    return cycles;
}
#endif /* PRT_CPUP_THREAD_INTERNAL_H */
