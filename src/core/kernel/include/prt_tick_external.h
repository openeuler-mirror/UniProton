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
 * Description: Tick私有头文件
 */
#ifndef PRT_TICK_EXTERNAL_H
#define PRT_TICK_EXTERNAL_H

#include "prt_tick.h"

/*
 * 模块间宏定义
 */
#define OS_TICKLESS_FOREVER ((U64)-1)

/*
 * 模块间typedef声明
 */
typedef void(*SwitchScanFunc)(void);
typedef void (*TskmonTickHook)(void);

extern SwitchScanFunc g_swtmrScanHook;
/* 任务检测Tick中断调用钩子 */
extern TskmonTickHook g_tskMonHook;

extern void OsTickDispatcher(void);
#if defined(OS_OPTION_SMP)
extern void OsTickForward(U32 coreMask);
extern void OsTickForwardISR(void);
#endif

extern U32 g_cyclePerTick;
OS_SEC_ALW_INLINE INLINE U32 OsGetCyclePerTick(void)
{
    return g_cyclePerTick;
}
#endif /* PRT_TICK_EXTERNAL_H */
