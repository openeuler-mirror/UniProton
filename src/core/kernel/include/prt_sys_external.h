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
 * Description: 系统信息的内部头文件
 */
#ifndef PRT_SYS_EXTERNAL_H
#define PRT_SYS_EXTERNAL_H

#include "prt_sys.h"
#include "prt_task.h"
#include "prt_sem.h"
#include "prt_tick.h"
#include "prt_list_external.h"
#include "prt_asm_cpu_external.h"
#include "prt_cpu_external.h"

#define OS_SYS_PID_BASE (OsGetHwThreadId() << OS_TSK_TCB_INDEX_BITS)

#define OS_INT_ACTIVE_MASK \
    (OS_FLG_HWI_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)

#define OS_INT_ACTIVE ((UNI_FLAG & OS_INT_ACTIVE_MASK) != 0)
#define OS_INT_INACTIVE (!(OS_INT_ACTIVE))
#define OS_HWI_ACTIVE_MASK  (OS_FLG_HWI_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)
#define OS_HWI_ACTIVE ((UNI_FLAG & OS_HWI_ACTIVE_MASK) != 0)

#define OS_THREAD_FLAG_MASK \
    (OS_FLG_HWI_ACTIVE | OS_FLG_BGD_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)

#define OS_SYS_TASK_STATUS(flag) (((flag) & OS_THREAD_FLAG_MASK) == OS_FLG_BGD_ACTIVE)
#define OS_SYS_HWI_STATUS(flag) (((flag) & OS_FLG_HWI_ACTIVE) != 0)

#define OS_MS2CYCLE(ms, clock) DIV64(((ms) * (U64)(clock)), OS_SYS_MS_PER_SECOND) /* 毫秒转换成cycle */
#define OS_US2CYCLE(us, clock) DIV64(((us) * (U64)(clock)), OS_SYS_US_PER_SECOND) /* 微秒转换成cycle */

/*
 * 模块间typedef声明
 */
typedef void (*TickDispFunc)(void);
/* 有TICK情况下CPU占用率触发函数类型定义。 */
typedef void (*TickEntryFunc)(void);
typedef void (*TaskScanFunc)(void);
typedef U64 (*SysTimeFunc)(void);
/*
 * 模块间全局变量声明
 */
extern U32 g_threadNum;

extern U32 g_tickNoRespondCnt;
#define TICK_NO_RESPOND_CNT g_tickNoRespondCnt

extern U32 g_systemClock;

extern struct TickModInfo g_tickModInfo;
extern U64 g_uniTicks;
extern TickEntryFunc g_tickTaskEntry;
extern TaskScanFunc g_taskScanHook;
extern TickDispFunc g_tickDispatcher;

extern U32 g_uniFlag;
#define UNI_FLAG g_uniFlag

/*
 * 模块间函数声明
 */
extern U8 OsGetCpuType(void);
extern U64 OsCurCycleGet64(void);
extern U32 OsSetSysTimeHook(SysTimeFunc hook);
extern enum SysThreadType OsCurThreadType(void);

/*
 * 模块间内联函数定义
 */
#define OS_TASK_SCAN()                \
    do {                              \
        if (g_taskScanHook != NULL) { \
            g_taskScanHook();         \
        }                             \
    } while (0)

#endif /* PRT_SYS_EXTERNAL_H */
