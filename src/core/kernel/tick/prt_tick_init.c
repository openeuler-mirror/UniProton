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
#include "prt_tick_internal.h"

/* Tick中断对应的硬件定时器ID */
OS_SEC_L4_DATA U16 g_tickHwTimerIndex = OS_MAX_U16;
OS_SEC_BSS U32 g_cyclePerTick;

/*
 * 描述：注册Tick中断模块信息
 */
#if defined(OS_OPTION_SMP)
OS_SEC_L4_TEXT U32 OsHwTickInitSmp(void)
{
    U32 ret;

    /* GIC寄存器私有问题 */
    ret = PRT_HwiSetAttr(OS_SMP_TICK_TRIGGER_OTHER_CORE_SGI,
                        OS_SMP_TICK_TRIGGER_OTHER_CORE_SGI_PRI, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(OS_SMP_TICK_TRIGGER_OTHER_CORE_SGI, (HwiProcFunc)OsTickForwardISR, 0);
    if (ret != OS_OK) {
        return ret;
    }

    (void)PRT_HwiEnable(OS_SMP_TICK_TRIGGER_OTHER_CORE_SGI);
    return ret;
}
#endif
OS_SEC_L4_TEXT U32 OsTickRegister(struct TickModInfo *modInfo)
{
    g_tickModInfo.tickPerSecond = modInfo->tickPerSecond;
    g_tickModInfo.tickPriority = modInfo->tickPriority;

    return OS_OK;
}

/*
 * 描述：Tick中断的处理函数
 */
OS_SEC_L0_TEXT void OsTickHookDispatcher(void)
{
    OS_MHOOK_ACTIVATE_PARA0(OS_HOOK_TICK_ENTRY);
    OsTickDispatcher();
    OS_MHOOK_ACTIVATE_PARA0(OS_HOOK_TICK_EXIT);
}

/*
 * 描述：初始化Tick中断模块
 */
OS_SEC_L4_TEXT U32 OsTickConfigInit(void)
{
    if ((g_tickModInfo.tickPerSecond == 0) || (g_tickModInfo.tickPerSecond > g_systemClock)) {
        return OS_ERRNO_TICK_PER_SECOND_ERROR;
    }

    g_cyclePerTick = g_systemClock / (g_tickModInfo.tickPerSecond);

    if (OS_IS_TICK_PERIOD_INVALID(g_cyclePerTick)) {
        return OS_ERRNO_TICK_PERIOD;
    }

    g_tickDispatcher = OsTickHookDispatcher;

    return OS_HW_TICK_INIT();
}

/*
 * 描述：启动Tick中断
 */
OS_SEC_L4_TEXT U32 OsTickStart(void)
{
    return OS_OK;
}

