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
 * Description: System base function implementation
 */
#include "prt_idle.h"
#include "prt_sys_external.h"
#include "prt_kexc_external.h"
#include "prt_hook_external.h"
#include "prt_sys.h"
#if defined(OS_OPTION_SMP)
#include "prt_sched_external.h"
#include "prt_buildef.h"
#endif

/* 初值非0的原因：初值为0在段属性为空场景下放到bss段初始化 */
OS_SEC_L4_DATA U8 g_maxNumOfCores = OS_MAX_CORE_NUM;
OS_SEC_L4_DATA U32 g_validAllCoreMask = (1U << (OS_MAX_CORE_NUM)) - 1;
OS_SEC_L4_BSS U8 g_numOfCores;

/* 系统主频 */
OS_SEC_BSS U32 g_systemClock;

/* cpup */
OS_SEC_BSS TickEntryFunc g_tickTaskEntry;

OS_SEC_BSS TickDispFunc g_tickDispatcher;
/* tick中软件定时器扫描钩子 */
OS_SEC_BSS TaskScanFunc g_taskScanHook;
OS_SEC_BSS struct TickModInfo g_tickModInfo;

OS_SEC_L4_BSS U32 g_threadNum;
/* Tick计数 */
OS_SEC_BSS U64 g_uniTicks;

OS_SEC_L4_BSS U8 g_primaryCoreId;

#if !defined(OS_OPTION_SMP)
/* 系统状态标志位 */
OS_SEC_DATA U32 g_uniFlag = 0;
OS_SEC_BSS U32 g_tickNoRespondCnt;
OS_SEC_DATA struct TagTskCb *g_runningTask = NULL;
#else
OS_SEC_BSS CoreWakeUpHook g_coreWakeupHook;
#endif

OS_SEC_ALW_INLINE INLINE enum SysThreadType OsCurThreadTypeNoIntLock(void)
{
    U32 uniFlag = UNI_FLAG;

    if (LIKELY(OS_SYS_TASK_STATUS(uniFlag))) {
        return SYS_TASK;
    } else if (OS_SYS_HWI_STATUS(uniFlag)) {
        return SYS_HWI;
    }

    return SYS_KERNEL; // 异常、tick、sys、初始化
}

/*
 * 描述：获取系统当前线程类型。
 */
OS_SEC_TEXT enum SysThreadType OsCurThreadType(void)
{
    enum SysThreadType type;
    uintptr_t intSave = OsIntLock();

    /* 异常模块打开而且目前系统有异常产生 */
    if (CUR_NEST_COUNT > 0) {
        OsIntRestore(intSave);
        return SYS_KERNEL;
    }

    type = OsCurThreadTypeNoIntLock();

    OsIntRestore(intSave);
    return type;
}

OS_SEC_L4_TEXT U32 PRT_IdleAddHook(IdleHook hook)
{
    return OsHookAdd(OS_HOOK_IDLE_PERIOD, (OsVoidFunc)hook);
}
OS_SEC_L4_TEXT U32 PRT_IdleDelHook(IdleHook hook)
{
    return OsHookDel(OS_HOOK_IDLE_PERIOD, (OsVoidFunc)hook);
}

#if defined(OS_OPTION_SMP)
OS_SEC_L0_TEXT enum SysThreadType PRT_CurThreadTypeNoIntLock(void)
{
    return OsCurThreadTypeNoIntLock();
}
#endif