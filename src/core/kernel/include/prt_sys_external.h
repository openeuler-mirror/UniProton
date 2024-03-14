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
#include "prt_sched_external.h"
#include "prt_task_external.h"

#if !defined(OS_OPTION_SMP)
#define OS_VAR_ARRAY_NUM 1
#else
#define OS_VAR_ARRAY_NUM OS_MAX_CORE_NUM
#endif

#define ARRAY_COUNT(arr) (sizeof(arr) / sizeof((arr)[0]))

#if defined(OS_OPTION_SMP)
#define THIS_CORE() (OsGetHwThreadId())
#else
#define THIS_CORE() OS_THIS_CORE
#endif

#if defined(OS_OPTION_SMP)
#define OS_SYS_PID_BASE (OsGetHwThreadId() << OS_TSK_TCB_INDEX_BITS)
#else
#define OS_SYS_PID_BASE (0x0U << OS_TSK_TCB_INDEX_BITS)
#endif

#if !defined(OS_OPTION_SMP)
extern U32 g_uniFlag;
#define UNI_FLAG g_uniFlag
#else
#define UNI_FLAG (THIS_RUNQ()->uniFlag)
extern CoreWakeUpHook g_coreWakeupHook;

OS_SEC_ALW_INLINE INLINE void OsCoreWakeupIpc(U32 coreID)
{
    CoreWakeUpHook wakeupHook = g_coreWakeupHook;

    if (wakeupHook != NULL) {
        wakeupHook(coreID);
    }
    return;
}
#endif

#define OS_INT_ACTIVE_MASK \
    (OS_FLG_HWI_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)

#define OS_INT_ACTIVE  ((UNI_FLAG & OS_INT_ACTIVE_MASK) != 0)
#define OS_INT_INACTIVE (!(OS_INT_ACTIVE))
#define OS_HWI_ACTIVE_MASK  (OS_FLG_HWI_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)
#define OS_HWI_ACTIVE ((UNI_FLAG & OS_HWI_ACTIVE_MASK) != 0)
#define OS_HWI_INACTIVE (!(OS_INT_ACTIVE))

#define OS_THREAD_FLAG_MASK \
    (OS_FLG_HWI_ACTIVE | OS_FLG_BGD_ACTIVE | OS_FLG_TICK_ACTIVE | OS_FLG_SYS_ACTIVE | OS_FLG_EXC_ACTIVE)

#define OS_SYS_TASK_STATUS(flag) (((flag) & OS_THREAD_FLAG_MASK) == OS_FLG_BGD_ACTIVE)
#define OS_SYS_HWI_STATUS(flag) (((flag) & OS_FLG_HWI_ACTIVE) != 0)

#define OS_MS2CYCLE(ms, clock) DIV64(((ms) * (U64)(clock)), OS_SYS_MS_PER_SECOND) /* 毫秒转换成cycle */
#define OS_US2CYCLE(us, clock) DIV64(((us) * (U64)(clock)), OS_SYS_US_PER_SECOND) /* 微秒转换成cycle */

#define OS_CORE_STR_MAX_LEN 4
#define OS_CORE_STR_END_INDEX   (OS_CORE_STR_MAX_LEN - 1)   // 结束字符位置索引
#define OS_CORE_STR_NUM_INDEX   (OS_CORE_STR_END_INDEX -1)  // 最后一个有效字符位置索引
/*
 * 模块间typedef声明
 */
typedef void (*TickDispFunc)(void);
/* 有TICK情况下CPU占用率触发函数类型定义。 */
typedef void (*TickEntryFunc)(void);
typedef void (*TaskScanFunc)(void);
/*
 * 模块间全局变量声明
 */
extern U32 g_threadNum;

#if defined(OS_OPTION_SMP)
#define TICK_NO_RESPOND_CNT (THIS_RUNQ()->tickNoRespondCnt)
#else
extern U32 g_tickNoRespondCnt;
#define TICK_NO_RESPOND_CNT g_tickNoRespondCnt
#endif

extern U32 g_systemClock;
extern U8 g_numOfCores;
extern U8 g_maxNumOfCores;
extern U32 g_validAllCoreMask;
extern U8 g_primaryCoreId;

OS_SEC_ALW_INLINE INLINE U32 OsSysGetClock(void)
{
    return g_systemClock;
}
extern struct TickModInfo g_tickModInfo;
extern U64 g_uniTicks;
extern TickEntryFunc g_tickTaskEntry;
extern TaskScanFunc g_taskScanHook;
extern TickDispFunc g_tickDispatcher;

struct CoreNumStr {
    char coreNo[OS_CORE_STR_MAX_LEN];
};
/*
 * 模块间函数声明
 */
extern U8 OsGetCpuType(void);
extern U64 OsCurCycleGet64(void);
extern U32 OsSetSysTimeHook(SysTimeFunc hook);
extern enum SysThreadType OsCurThreadType(void);
#if defined(OS_OPTION_SMP)
extern void OsGetCoreStr(struct CoreNumStr *str);
#endif
/*
 * 模块间内联函数定义
 */
#define OS_TASK_SCAN()                \
    do {                              \
        if (g_taskScanHook != NULL) { \
            g_taskScanHook();         \
        }                             \
    } while (0)

OS_SEC_ALW_INLINE INLINE U32 OsSysGetTickPerSecond(void)
{
    return g_tickModInfo.tickPerSecond;
}
#endif /* PRT_SYS_EXTERNAL_H */
