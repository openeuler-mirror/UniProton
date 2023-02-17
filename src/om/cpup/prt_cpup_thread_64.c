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
 * Description: 线程级CPU占用率模块的C文件
 */
#include "prt_cpup_thread_internal.h"

/*
 * 描述：任务切入，中断结束时，统计任务的startTime
 */
OS_SEC_L2_TEXT void OsNowTskCycleStart(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

    /* 处理硬中断、Tick退出钩子时判断是否需要统计CPUP */
    if (CPUP_FLAG == OS_CPUP_EXIT_FLAG) {
        OS_TASK_CYCLE_START(RUNNING_TASK->taskPid, OsCurCycleGet64());
    }
    CPUP_FLAG--;

    OsIntRestore(intSave);
}

/*
 * 描述：任务切出，中断开始时，统计任务的allTime
 */
OS_SEC_L2_TEXT void OsNowTskCycleEnd(void)
{
    uintptr_t intSave;

    intSave = OsIntLock();

    /* 处理硬中断、Tick进入钩子时判断是否需要统计CPUP */
    if (CPUP_FLAG == OS_CPUP_ENTRY_FLAG) {
        OS_TASK_CYCLE_END(RUNNING_TASK->taskPid, OsCurCycleGet64());
    }
    CPUP_FLAG++;

    OsIntRestore(intSave);
}

/*
 * 描述：第一次任务切换时候CPUP设初始值
 */
OS_SEC_L2_TEXT void OsCpupFirstSwitch(void)
{
    g_cpuWinStart = OsCurCycleGet64();
    OS_TASK_CYCLE_START(g_highestTask->taskPid, g_cpuWinStart);
}

/*
 * 描述：任务结束和开始，对任务的CPUP计时
 * 备注：lastTaskId 结束的任务ID；nextTaskId 开始的任务ID；curCycle  结束和开始的Cycle
 */
OS_SEC_L2_TEXT void OsCpupStartEnd(U32 lastTaskId, U32 nextTaskId, U64 curCycle)
{
    /* 切出任务 */
    OS_TASK_CYCLE_END(lastTaskId, curCycle);

    /* 切入任务 */
    OS_TASK_CYCLE_START(nextTaskId, curCycle);
}

/*
 * 描述：任务切换时候CPUP设初始值
 */
OS_SEC_L2_TEXT void OsCpupTskSwitch(U32 lastTaskId, U32 nextTaskId)
{
    uintptr_t intSave;

    intSave = OsIntLock();
    /* CPUP统计 */
    OsCpupStartEnd(lastTaskId, nextTaskId, OsCurCycleGet64());

    OsIntRestore(intSave);
}
/*
 * 描述：获取输入线程个数的cpup
 */
OS_SEC_L2_TEXT U32 OsCpupTask(U32 intNum, struct CpupThread *cpup)
{
    U32 index;
    U32 maxNum = 0;
    cpup[maxNum].id = OS_CPUP_INT_ID;
    cpup[maxNum].usage = OsCpupIntGet();
    maxNum++;

    for (index = 0; index < (OS_MAX_TCB_NUM - 1); index++) {
        if (intNum == maxNum) {
            break;
        }

        /* 判断该任务是否被创建 */
        if (g_tskCbArray[index].taskStatus == OS_TSK_UNUSED) {
            continue;
        }

        cpup[maxNum].id = g_tskCbArray[index].taskPid;
        cpup[maxNum].usage = g_cpup[index].usage;

        maxNum++;
    }
    return maxNum;
}

OS_SEC_ALW_INLINE INLINE U32 OsCpupParaCheck(U32 intNum, struct CpupThread *cpup, const U32 *outNum)
{
    if (cpup == NULL || outNum == NULL) {
        return OS_ERRNO_CPUP_PTR_NULL;
    }

    if (intNum == 0) {
        return OS_ERRNO_CPUP_THREAD_INNUM_INVALID;
    }

    return OS_OK;
}

/*
 * 描述：获取所有任务线程的运行时间
 */
OS_SEC_L2_TEXT U64 OsCpupAllTaskTimeGet(void)
{
    U32 index;
    U64 allTime = 0;

    for (index = 0; index < (OS_MAX_TCB_NUM - 1); index++) {
        allTime += g_cpup[index].allTime;
    }

    return (allTime + g_cpuTimeDelTask);
}

/*
 * 描述：清空所有任务运行时间和CPU占用率大小
 */
OS_SEC_L2_TEXT void OsCpupTimeClear(void)
{
    U32 index;

    for (index = 0; index < (OS_MAX_TCB_NUM - 1); index++) {
        g_cpup[index].allTime = 0;
    }

    g_cpuTimeDelTask = 0;
}
/*
 * 描述：清空所有任务运行时间和CPU占用率大小
 */
OS_SEC_L2_TEXT void OsCpupTickCal(void)
{
    U32 index;

    for (index = 0; index < (OS_MAX_TCB_NUM - 1); index++) {
        g_cpup[index].usage = (U16)DIV64(g_cpup[index].allTime * CPUP_USE_RATE, g_baseValue);
        if (g_cpup[index].usage > CPUP_USE_RATE) {
            g_cpup[index].usage = CPUP_USE_RATE;
        }
    }

    OsMcCpupSet(0x0U, (U32)(CPUP_USE_RATE - g_cpup[TSK_GET_INDEX(IDLE_TASK_ID)].usage));

    g_cpupDelTask = (U16)DIV64(g_cpuTimeDelTask * CPUP_USE_RATE, g_baseValue);
}

/*
 * 描述：获取中断线程的占用率接口
 */
OS_SEC_L2_TEXT U16 OsCpupIntGet(void)
{
    U32 index;
    U16 usage = 0;

    for (index = 0; index < (OS_MAX_TCB_NUM - 1); index++) {
        usage += g_cpup[index].usage;
    }

    usage += g_cpupDelTask;

    if (usage >= CPUP_USE_RATE) {
        usage = CPUP_USE_RATE;
    }

    return (U16)(CPUP_USE_RATE - usage);
}

/*
 * 描述：获取当前线程级cpu占用率接口
 */
OS_SEC_L2_TEXT U32 OsCpupThreadNow(void)
{
    U64 cpuCycleAll;
    uintptr_t intSave;
    U32 cpup = 0;
    U64 curCycle;

    if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) == 0) {
        OS_REPORT_ERROR(OS_ERRNO_CPUP_OS_NOT_STARTED);
        return (U32)OS_INVALID;
    }

    intSave = OsIntLock();

    if (g_ticksPerSample != 0) {
        cpup = (U32)(CPUP_USE_RATE - g_cpup[TSK_GET_INDEX(IDLE_TASK_ID)].usage);

        OsIntRestore(intSave);
        return cpup;
    }

    /* 统计当前系统运行总时间 */
    curCycle = OsCurCycleGet64();

    if (OS_INT_INACTIVE) {
        OsCpupStartEnd(RUNNING_TASK->taskPid, RUNNING_TASK->taskPid, curCycle);
    }

    cpuCycleAll = OsCpupGetWinCycles(curCycle);
    g_cpuWinStart = curCycle;

    cpup = (U32)(CPUP_USE_RATE - DIV64(CPUP_USE_RATE * g_cpup[TSK_GET_INDEX(IDLE_TASK_ID)].allTime, cpuCycleAll));
    OsMcCpupSet(0x0U, cpup);

    OsCpupTimeClear();

    OsIntRestore(intSave);

    return cpup;
}

/*
 * 描述：获取所有线程的占用率接口
 */
OS_SEC_L2_TEXT U32 PRT_CpupThread(U32 inNum, struct CpupThread *cpup, U32 *outNum)
{
    U32 index;
    U32 ret;
    uintptr_t intSave;
    U32 maxNum = 1; // 默认中断占用一个

    U64 cpuCycleAll;
    U64 allTime;
    U64 curCycle;

    ret = OsCpupPreCheck();
    if (ret != OS_OK) {
        return ret;
    }

    ret = OsCpupParaCheck(inNum, cpup, outNum);
    if (ret != OS_OK) {
        return ret;
    }

    /* 获取当前采样周期内，所有线程运行的总时间 */
    intSave = OsIntLock();

    if (g_ticksPerSample != 0) {
        maxNum = OsCpupTask(inNum, cpup);
        OsIntRestore(intSave);
        *outNum = maxNum;

        return OS_OK;
    }

    curCycle = OsCurCycleGet64();

    if (OS_INT_INACTIVE) {
        OsCpupStartEnd(RUNNING_TASK->taskPid, RUNNING_TASK->taskPid, curCycle);
    }
    /* 统计当前系统运行总时间 */
    cpuCycleAll = OsCpupGetWinCycles(curCycle);
    g_cpuWinStart = curCycle;

    /*
     * 配置g_cpup[].iD线程ID
     */
    cpup[0].id = OS_CPUP_INT_ID;
    cpup[0].usage = (U16)DIV64(CPUP_USE_RATE * (cpuCycleAll - OsCpupAllTaskTimeGet()), cpuCycleAll);

    for (index = 0; index < (OS_MAX_TCB_NUM - 1) && maxNum < inNum; index++) {
        /* 判断该任务是否被创建 */
        if (g_tskCbArray[index].taskStatus == OS_TSK_UNUSED) {
            continue;
        }

        allTime = g_cpup[index].allTime;

        cpup[maxNum].id = g_tskCbArray[index].taskPid;
        cpup[maxNum].usage = (U16)DIV64(CPUP_USE_RATE * allTime, cpuCycleAll);

        maxNum++;
    }

    OsMcCpupSet(0x0U,
                (U32)DIV64(CPUP_USE_RATE * (cpuCycleAll - g_cpup[TSK_GET_INDEX(IDLE_TASK_ID)].allTime), cpuCycleAll));

    OsCpupTimeClear();

    OsIntRestore(intSave);
    *outNum = maxNum;

    return OS_OK;
}
