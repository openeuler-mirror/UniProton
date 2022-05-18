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
 * Description: CPU占用率模块的C文件
 */
#include "prt_cpup_internal.h"
#include "prt_cpup_thread_internal.h"

/* 数据定义放到XXX.c中是为了解决SAI报Data Module */
#if defined(OS_OPTION_CPUP_WARN)
/* CPUP告警检测函数钩子 */
OS_SEC_BSS CpupWarnFunc g_cpupWarnCheck;
#endif

OS_SEC_BSS CpupNowFunc g_cpupNow;
/* 系统级CPUP一个计算周期的的tick数 */
OS_SEC_BSS U32 g_ticksPerSample;
/* 当前系统级CPUP计数 */
OS_SEC_BSS U32 g_cpupIndex;
/* 线程级CPU占用率结构体指针 */
OS_SEC_BSS struct TagCpupThread *g_cpup;

/* 一个采样周期内，当前Tick计数值 */
OS_SEC_BSS U32 g_tickCount;
/* CPU占用率采样周期值 */
OS_SEC_BSS U64 g_baseValue;

OS_SEC_L4_TEXT U32 OsCpupRegister(struct CpupModInfo *modInfo)
{
    U32 ret;

    ret = OsCpupReg(modInfo);
    if (ret != OS_OK) {
        return ret;
    }

#if defined(OS_OPTION_CPUP_WARN)
    if (modInfo->cpupWarnFlag == TRUE) {
        ret = OsCpupWarnReg(modInfo);
        if (ret != OS_OK) {
            return ret;
        }
    }
#endif

    return OS_OK;
}

/*
 * 描述：模块间接口，获取CPUP是否已经初始化。
 */
OS_SEC_L4_TEXT bool OsCpupInitIsDone(void)
{
    if (g_cpupNow == NULL) {
        return FALSE;
    }
    return TRUE;
}

/*
 * 描述：根据g_cpupThreadInitFlag判断是否已经初始化CPUP模块如果没有，则进行初始化
 */
OS_SEC_L2_TEXT U32 OsCpupLazyInit(void)
{
    if (g_cpupNow == NULL) {
        return OS_ERRNO_CPUP_NOT_INITED;
    }
    return OS_OK;
}
