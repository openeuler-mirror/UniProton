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
 * Description: hw模块内部头文件。
 */
#ifndef PRT_HW_TICK_INTERNAL_H
#define PRT_HW_TICK_INTERNAL_H

#include "prt_lib_external.h"
#include "prt_sys_external.h"
#include "prt_tick_external.h"

/* 系统当前运行的时间，单位cycle */
extern U64 g_cycleNow;
/* 在tick中记录的系统当前cycle值 */
extern U64 g_cycleInTick;
/* 系统当前运行的时间，时间是g_cyclePerTick的整数倍 */
extern U64 g_cycleByTickNow;

/*
 * 模块内宏定义
 */
#define OS_SYSTICK_CONTROL_COUNTFLAG_MSK (1U << 16)
#define OS_SYSTICK_CONTROL_REG 0xE000E010
#define OS_SYSTICK_RELOAD_REG 0xE000E014
#define OS_SYSTICK_CURRENT_REG 0xE000E018

/*
 * 模块间函数声明
 */
extern void OsCycleUpdate(void);

#endif /* PRT_HW_TICK_INTERNAL_H */
