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

/* 当前cpu占用率统计时间窗起始时间 */
OS_SEC_BSS U64 g_cpuWinStart;
/* 在计算线程级CPUP时，创建但已删除任务的CPU时间总大小 */
OS_SEC_BSS U64 g_cpuTimeDelTask;
OS_SEC_BSS U16 g_cpupDelTask;

/* 硬中断、Tick钩子是否需要计算CPUP标识 */
OS_SEC_BSS U32 g_cpupFlag;

OS_SEC_L2_TEXT U32 OsCpupGet(void)
{
    return (U32)(CPUP_USE_RATE - g_cpup[TSK_GET_INDEX(IDLE_TASK_ID)].usage);
}

/*
 * 描述：有tick模式下的cpu占用率采样，在tick中断中被调用,每个tick调用一次，当到达采样间隔时间时计算cpu
 *       占用率，本函数放在tick处理函数中被调用
 */
OS_SEC_L2_TEXT void OsCpupThreadTickTask(void)
{
    /* 采样间隔，单位tick */
    U32 ticksPerSample = g_ticksPerSample;

    /* 每个tick调用一次，当tick数到达采样间隔时计算cpu占用率，全程关中断 */
    g_tickCount++;

    if (g_tickCount >= ticksPerSample) {
        OsCpupTickCal();

#if defined(OS_OPTION_CPUP_WARN)
        /* cpup告警检测 */
        if (g_cpupWarnCheck != NULL) {
            g_cpupWarnCheck();
        }
#endif

        g_tickCount -= ticksPerSample;

        OsCpupTimeClear();
    }
}

/*
 * 描述：参数检查接口，返回OS_OK成功，其他为错误码
 */
OS_SEC_L2_TEXT U32 OsCpupPreCheck(void)
{
    if (!OsCpupInitIsDone()) {
        return OS_ERRNO_CPUP_NOT_INITED;
    }

    if ((UNI_FLAG & OS_FLG_BGD_ACTIVE) == 0) {
        return OS_ERRNO_CPUP_OS_NOT_STARTED;
    }

    return OS_OK;
}
