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
 * Description: cpu占用率模块的内部公共头文件
 */
#ifndef PRT_CPUP_EXTERNAL_H
#define PRT_CPUP_EXTERNAL_H

#include "prt_cpup.h"

/*
 * 设置cpu占用率的注册信息结构体。
 */
struct TagOsCpupWarnInfo {
    /* CPU占用率告警阈值 */
    U32 warn;
    /* CPU占用率告警恢复阈值 */
    U32 resume;
};

/*
 * 线程级CPU占用率结构体
 */
struct TagCpupThread {
    /* 运行总时间记录 */
    U64 allTime;
    /* 调用前时间记录 */
    U64 startTime;
    /* CPU占用率 */
    U16 usage;
    /* 保留 */
    U16 reserve;
    /* 保留，64位对齐(R8 cacheline)> */
    U32 reserve2;
};

#define OS_CPUP_INT_ID 0xffffffff /* 中断线程ID */
#define CPUP_USE_RATE 10000 /* CPUP使用比率 10000:万分比  1000:千分比  100:百分比 */

typedef void (*CpupWarnFunc)(void);
typedef U32(*CpupNowFunc)(void);
typedef void (*CpupCoreSleepFunc)(void);

extern struct TagOsCpupWarnInfo g_cpupWarnInfo;
extern struct TagCpupThread *g_cpup;
extern U16 g_sysUsage;

extern bool OsCpupInitIsDone(void);
extern U32 OsCpupLazyInit(void);

/* 核休眠钩子函数 */
extern volatile CpupCoreSleepFunc g_cpupCoreSleep;

#if defined(OS_OPTION_CPUP)
/* CPUP核休眠钩子注册 */
#define OS_CPUP_CORE_SLEEP_HOOK_SET(handle) \
    do {                                    \
        if (OsCpupInitIsDone()) {           \
            g_cpupCoreSleep = (handle);     \
        }                                   \
    } while (0)
#else
#define OS_CPUP_CORE_SLEEP_HOOK_SET(handle)
#endif

#endif /* PRT_CPUP_EXTERNAL_H */
