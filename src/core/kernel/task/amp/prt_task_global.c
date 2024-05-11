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
 * Description: 任务切换钩子独立文件
 */
#include "prt_task_internal.h"

OS_SEC_L4_BSS struct TskModInfo g_tskModInfo;

OS_SEC_BSS TskEntryFunc g_tskIdleEntry;
/* 任务中创建删除时调用钩子 */
OS_SEC_BSS TaskNameAddFunc g_taskNameAdd;
OS_SEC_BSS TaskNameGetFunc g_taskNameGet;
OS_SEC_BSS volatile TskCoresleep g_taskCoreSleep;

OS_SEC_BSS U32 g_tskMaxNum;
OS_SEC_BSS struct TagTskCb *g_tskCbArray;
OS_SEC_BSS U32 g_tskBaseId;

OS_SEC_BSS TskHandle g_idleTaskId;
OS_SEC_BSS U16 g_uniTaskLock;
OS_SEC_BSS struct TagTskCb *g_highestTask;

#if defined(OS_OPTION_POWEROFF)
OS_SEC_BSS PowerOffFuncT g_sysPowerOffHook;
OS_SEC_BSS SetOfflineFlagFuncT g_setOfflineFlagHook = SetOfflineFlagDefaultFunc;
#endif

OS_SEC_TEXT void OsTskSwitchHookCaller(U32 prevPid, U32 nextPid)
{
    UNI_FLAG |= OS_FLG_SYS_ACTIVE;
    OS_MHOOK_ACTIVATE_PARA2(OS_HOOK_TSK_SWITCH, prevPid, nextPid);
    UNI_FLAG &= ~OS_FLG_SYS_ACTIVE;
}

OS_SEC_L4_TEXT U32 OsTskMaxNumGet(void)
{
    return g_tskMaxNum;
}
