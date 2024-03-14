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
 * Create: 2024-01-25
 * Description: Tick interrupt implementation
 */
#include "prt_tick_internal.h"
/*
 * 描述：获取每秒中拥有的tick数
 */
OS_SEC_L2_TEXT U32 PRT_TickPerSecondGet(void)
{
    return g_tickModInfo.tickPerSecond;
}

#if defined(OS_OPTION_SMP)
/*
 * 描述：触发本核tick后处理（bottom half）
 */
OS_SEC_TEXT void OsTickTailProcActivate(void)
{
#if !defined(OS_OPTION_TICK_USE_HWTMR)
    if (g_tickModInfo.tickPerSecond != 0) {
        TICK_NO_RESPOND_CNT++;
    }
#endif
}
/*
 * 描述：TICK转发中断处理函数
 */
OS_SEC_TEXT void OsTickForwardISR(void)
{
    OsTickTailProcActivate();
}
/*
 * 描述：转发tick中断
 */
OS_SEC_TEXT void OsTickForward(U32 coreMask)
{
    OsHwiMcTrigger(OS_TYPE_TRIGGER_BY_MASK, coreMask, OS_SMP_TICK_TRIGGER_OTHER_CORE_SGI);
}

#endif