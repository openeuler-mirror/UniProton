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
#ifndef PRT_CPUP_INTERNAL_H
#define PRT_CPUP_INTERNAL_H

#include "prt_lib_external.h"
#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_cpup_external.h"
#include "prt_mem_external.h"
#include "prt_sys_external.h"
#include "prt_task_external.h"
#include "prt_hook_external.h"

/*
 * 模块内宏定义
 */
extern U32 *g_cpupInstant;
extern U32 g_ticksPerSample;
extern U32 g_cpupIndex;
extern U32 g_tickCount;
extern U64 g_baseValue;
extern CpupNowFunc g_cpupNow;
/* CPUP告警检测函数钩子 */
extern CpupWarnFunc g_cpupWarnCheck;

/*
 * 模块内函数声明
 */
extern U32 OsCpupReg(struct CpupModInfo *modInfo);
extern U32 OsCpupWarnReg(struct CpupModInfo *modInfo);
extern U32 OsCpupInit(void);
/* 放在tick任务中 */
extern void OsCpupWarn(void);

extern U32 OsCpupGet(void);

OS_SEC_ALW_INLINE INLINE void OsMcCpupSet(U32 coreId, U32 cpupValue)
{
    (void)coreId;
    (void)cpupValue;
}

#endif /* PRT_CPUP_INTERNAL_H */
