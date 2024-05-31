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
 * Description: 线程级CPU占用率模块初始化文件
 */
#include "prt_cpup_thread_internal.h"

#if defined(OS_OPTION_TICKLESS)
/*
 * 描述: 获取Cpup下次计算的Tick刻度
 */
OS_SEC_TEXT U64 OsCpupNextTickGet(U32 coreId)
{
    (void)coreId;
    if (g_ticksPerSample > 0) {
        return g_cpupNextTick;
    }

    return OS_TICKLESS_FOREVER;
}

/*
 * 描述: 查看当前tick是否要统计一次Cpup，返回TRUE或者FALSE
 */
OS_SEC_TEXT bool OsCpupTargetCheck(void)
{
    if (g_ticksPerSample > 0) {
        if (g_cpupNextTick != OS_TICKLESS_FOREVER) {
            if (g_cpupNextTick <= g_uniTicks) {
                return TRUE;
            }
        }
    }

    return FALSE;
}

#endif

OS_SEC_L4_TEXT U32 OsCpupReg(struct CpupModInfo *modInfo)
{
    if (modInfo->cpupWarnFlag == TRUE) {
        if (modInfo->sampleTime == 0) {
            return OS_ERRNO_CPUP_SAMPLE_TIME_ZERO;
        }
    }

    OsMhookReserve((U32)OS_HOOK_FIRST_TIME_SWH, 1);
    OsMhookReserve((U32)OS_HOOK_TSK_SWITCH, 1);
    OsMhookReserve((U32)OS_HOOK_HWI_ENTRY, 1);
    OsMhookReserve((U32)OS_HOOK_HWI_EXIT, 1);
    OsMhookReserve((U32)OS_HOOK_TICK_ENTRY, 1);
    OsMhookReserve((U32)OS_HOOK_TICK_EXIT, 1);

    g_ticksPerSample = modInfo->sampleTime;
    return OS_OK;
}
/*
 * 描述：tick统计所需要的函数变量初始化。
 */
OS_SEC_L4_TEXT void OsCpupGlobalInit(void)
{
    if (g_ticksPerSample != 0) {
#if defined(OS_OPTION_TICKLESS)
        /* 这里首次计算CPUP的时刻为一个采样周期的tick刻度 */
        g_cpupNextTick = g_ticksPerSample;
        g_getCpupNearestTick = OsCpupNextTickGet;
        g_checkCpupTickProcess = (CheckTickProcessFunc)OsCpupTargetCheck;
#endif
        g_tickTaskEntry = (TickEntryFunc)OsCpupThreadTickTask;
    }

    g_cpupNow = OsCpupThreadNow;
}

OS_SEC_L4_TEXT U32 OsCpupThreadHookAdd(void)
{
    U32 ret;

    /* CPUP统计在第一次任务切换的时候 */
    ret = OsMhookAdd((U32)OS_HOOK_FIRST_TIME_SWH, (OsVoidFunc)OsCpupFirstSwitch);
    if (ret != OS_OK) {
        return ret;
    }

    /* CPUP统计在任务切换的时候 */
    ret = OsMhookAdd((U32)OS_HOOK_TSK_SWITCH, (OsVoidFunc)OsCpupTskSwitch);
    if (ret != OS_OK) {
        return ret;
    }

    /* CPUP统计在进入硬中断的时候 */
    ret = OsMhookAdd((U32)OS_HOOK_HWI_ENTRY, (OsVoidFunc)OsNowTskCycleEnd);
    if (ret != OS_OK) {
        return ret;
    }

    /* CPUP统计在退出硬中断的时候 */
    ret = OsMhookAdd((U32)OS_HOOK_HWI_EXIT, (OsVoidFunc)OsNowTskCycleStart);
    if (ret != OS_OK) {
        return ret;
    }

    /* CPUP统计在进入Tick的时候 */
    ret = OsMhookAdd((U32)OS_HOOK_TICK_ENTRY, (OsVoidFunc)OsNowTskCycleEnd);
    if (ret != OS_OK) {
        return ret;
    }

    /* CPUP统计在退出Tick的时候 */
    ret = OsMhookAdd((U32)OS_HOOK_TICK_EXIT, (OsVoidFunc)OsNowTskCycleStart);
    if (ret != OS_OK) {
        return ret;
    }

    return OS_OK;
}

/*
 * 描述：线程级CPUP初始化接口，返回OS_OK或错误码
 */
OS_SEC_L4_TEXT U32 OsCpupInit(void)
{
    U32 ret;
    U32 size;

    /* 多申请一个任务线程CPUP信息结构体大小，预留给第一次任务切换时切出的无效任务使用，防止溢出 */
    size = (OS_MAX_TCB_NUM) * sizeof(struct TagCpupThread);
    g_cpup = (struct TagCpupThread *)OsMemAllocAlign((U32)OS_MID_CPUP, OS_MEM_DEFAULT_FSC_PT,
                                                     size, MEM_ADDR_ALIGN_032);
    if (g_cpup == NULL) {
        return OS_ERRNO_CPUP_NO_MEMORY;
    }

    if (memset_s(g_cpup, size, 0, size) != EOK) {
        OS_GOTO_SYS_ERROR1();
    }

    g_baseValue = (g_systemClock / g_tickModInfo.tickPerSecond) * (U64)g_ticksPerSample;

    OsCpupGlobalInit();

    ret = OsCpupThreadHookAdd();
    return ret;
}
