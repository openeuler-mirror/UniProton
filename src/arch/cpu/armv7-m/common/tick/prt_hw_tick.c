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
 * Description: Tick interrupt implementation
 */
#include "prt_hw_tick_internal.h"

/* 系统当前运行的时间，单位cycle */
OS_SEC_BSS U64 g_cycleNow;
/* 在tick中记录的系统当前cycle值 */
OS_SEC_BSS U64 g_cycleInTick;
/* 系统当前运行的时间，时间是g_cyclePerTick的整数倍 */
OS_SEC_BSS U64 g_cycleByTickNow;

OS_SEC_TEXT void OsTickIsr(void)
{
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    TICK_NO_RESPOND_CNT++;
    /* 调用CycleUpdate更新g_cycleNow g_cycleByTickNow */
    OsCycleUpdate();
    PRT_HwiRestore(intSave);
    OsHwiTrap();
}

/*
 * 描述：设置Tick硬件定时器定时时长，使能Tick中断定时器
 */
OS_SEC_TEXT void OsTickStartRegSet(U16 tickHwTimerIndex, U32 cyclePerTick)
{
    (void)tickHwTimerIndex;
    g_cycleByTickNow = 0;
    g_cycleInTick = 0;
    TICK_NO_RESPOND_CNT = 0;
    /* 系统初始化的时候将其初始化为0，因为初始化时为关中断，不会响应tick中断，只有在切换到多线程的模式才会开始计数 */
    g_uniTicks = 0;

    /* M3时钟计数寄存器，当计数到0时，该值会被重新装载 */
    *(volatile U32 *)OS_SYSTICK_RELOAD_REG = cyclePerTick;
    /*
     * M4时钟控制寄存器: bit0为使能位，1表示使能；bit1为计数减为0
     * 时是否产生中断源，1表示产生中断；bit2为使用内部时钟还是外部时钟，1表示未内部时钟
     */
    *(volatile U32 *)OS_SYSTICK_CONTROL_REG = OS_BIT0_MASK | OS_BIT1_MASK | OS_BIT2_MASK;
}

OS_SEC_L4_TEXT U32 OsTickTimerStartMx(U32 cyclePerTick)
{
    OsTickStartRegSet(0, cyclePerTick);
    return OS_OK;
}
