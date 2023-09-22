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
#if defined(OS_OPTION_LINUX)
#include <linux/jiffies.h>
#endif

/* 任务检测Tick中断调用钩子 */
OS_SEC_BSS TskmonTickHook g_tskMonHook;
/* 软件定时器扫描钩子 */
OS_SEC_BSS SwitchScanFunc g_swtmrScanHook;
OS_SEC_BSS TickHandleFunc g_tickUsrHook;

#if defined(OS_OPTION_LINUX)
unsigned long volatile jiffies;
#endif
/*
 * 描述：Tick中断的处理函数。扫描任务超时链表、扫描超时软件定时器、扫描TSKMON等。
 */
OS_SEC_TEXT void OsTickDispatcher(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

    g_uniTicks++;
#if defined(OS_OPTION_LINUX)
    jiffies = g_uniTicks;
#endif

    OS_TICK_COUNT_UPDATE();

    // 任务超时扫描
    OS_TASK_SCAN();

    // tick模式下的cpu占用率采样
    OS_TICK_TASK_ENTRY();

    OsIntRestore(intSave);

    TICK_USER_HOOK_RUN();

    OS_SWTMR_SCAN();
    TSKMON_TICK_RUN();
}

void PRT_TickISR(void)
{
#if !defined(OS_OPTION_TICK_USE_HWTMR)
    if (OsSysGetTickPerSecond() != 0) {
        TICK_NO_RESPOND_CNT++;
    }
#endif
}