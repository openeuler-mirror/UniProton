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
 * Description: 硬件tick模块的C文件。
 */
#include "prt_hw_tick_internal.h"

/*
 * 描述：CycleUpdate内部函数，该接口必须关中断情况下调用
 */
OS_SEC_L2_TEXT void OsCycleUpdate(void)
{
    U32 hwCycleFirst;  /* PosA */
    U32 hwCycleCtrl;   /* PosB */
    U32 hwCycleSecond; /* PosC */
    /*
     * 此处关中断后SysTick 中断上报会发生在以上四个时机
     * 1), Before  PosA then CONTROL_COUNTFLAG will be set and hwCycleFirst >= hwCycleSecond
     * 2), Between PosA and PosB then CONTROL_COUNTFLAG will be set and hwCycleFirst < hwCycleSecond
     * 3), Between PosB and PosC then CONTROL_COUNTFLAG will be clear and hwCycleFirst < hwCycleSecond
     * 4), After PosC we'll see it next time
     */
    hwCycleFirst = *(volatile U32 *)OS_SYSTICK_CURRENT_REG;
    hwCycleCtrl = *(volatile U32 *)OS_SYSTICK_CONTROL_REG;
    hwCycleSecond = *(volatile U32 *)OS_SYSTICK_CURRENT_REG;

    /* 如果1、装载值前一次比后一次小 或 2、OS_SYSTICK_CONTROL_COUNTFLAG 已经SET， */
    /* 则 说明tick反转，需要手动补偿一个周期 */
    if (((hwCycleCtrl & OS_SYSTICK_CONTROL_COUNTFLAG_MSK) != 0) || (hwCycleFirst < hwCycleSecond)) {
        g_cycleByTickNow += OsGetCyclePerTick();
        /* 再一次读时钟定时器，读清COUNTFLAG位 */
        /* COUNTFLAG位(bit16)：Returns 1 If Timer Counted To 0 Since Last Time This Was Read */
        (void)*(volatile U32 *)OS_SYSTICK_CONTROL_REG;
    }
    g_cycleNow = (g_cycleByTickNow + (OsGetCyclePerTick() - hwCycleSecond));
}

OS_SEC_L2_TEXT U64 PRT_ClkGetCycleCount64(void)
{
    U64 cycle;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    OsCycleUpdate();
    cycle = g_cycleNow;
    PRT_HwiRestore(intSave);
    return cycle;
}
