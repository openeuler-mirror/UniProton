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
 * Description: 中断模块内部头文件
 */
#ifndef PRT_IRQ_EXTERNAL_H
#define PRT_IRQ_EXTERNAL_H

#include "prt_hwi_external.h"
#if defined(OS_OPTION_SMP)
#include "prt_sched_external.h"
#endif
/* 无效的线程控制块地址 */
#define OS_HWI_INVALID_TCB_ADDR 0xffffffff
/* 模块内全局变量声明 */
#if defined(OS_OPTION_SMP)
#define OS_INT_COUNT (THIS_RUNQ()->intCount)
#else
extern U32 g_intCount;
#define OS_INT_COUNT g_intCount
#endif
/* Tick中断对应的硬件定时器ID */
extern U16 g_tickHwTimerIndex;

#if defined(OS_OPTION_SMP)
struct TagHwiHandleForm {
    union {
        HwiProcFunc hook;               // 非私有中断时为正常hook
        struct TagHwiHandleForm *form;  // 私有中断时为指向私有form数组的指针
    };

#if defined(OS_OPTION_HWI_ATTRIBUTE)
    HwiArg param;
#endif
};

struct TagHwiModeForm {
    HwiMode mode;
    HwiPrior prior;
#if defined(OS_OPTION_HWI_AFFINITY)
    U32 affinityMask;
#endif
};
#endif

/* 模块内函数声明 */
extern void OsHwiDefaultHandler(HwiArg arg);
extern void OsHwiHookDispatcher(HwiHandle archHwi);
extern void OsHwiCombineDispatchHandler(HwiArg arg);

extern void OsHwiDispatchTail(void);

#endif /* PRT_IRQ_EXTERNAL_H */
