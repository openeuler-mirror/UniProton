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
 * Description: UniProton配置头文件，裁剪开关和配置项。
 */
#ifndef PRT_CONFIG_H
#define PRT_CONFIG_H

#include "prt_buildef.h"
#include "prt_typedef.h"

extern char rmem_start[];


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

/* ***************************** 配置系统基本信息 ******************************* */
/* 芯片主频 */
#define OS_SYS_CLOCK                                    10000000ull
/* 用户注册的获取系统时间的函数*/
#define OS_SYS_TIME_HOOK                                NULL

/* ***************************** 中断模块配置 ************************** */
/* 硬中断最大支持个数 */
#define OS_HWI_MAX_NUM_CONFIG                           150

/* ***************************** 配置Tick中断模块 *************************** */
/* Tick中断模块裁剪开关 */
#define OS_INCLUDE_TICK                                 YES
/* Tick中断时间间隔，tick处理时间不能超过1/OS_TICK_PER_SECOND(s) */
#define OS_TICK_PER_SECOND                              10000

/* ***************************** 配置定时器模块 ***************************** */
/* 基于TICK的软件定时器裁剪开关 */
#define OS_INCLUDE_TICK_SWTMER                          YES
/* 基于TICK的软件定时器最大个数 */
#define OS_TICK_SWITIMER_MAX_NUM                        8

/* ***************************** 配置任务模块 ******************************* */
/* 任务模块裁剪开关 */
#define OS_INCLUDE_TASK                                 YES
/* 最大支持的任务数,最大共支持254个 */
#define OS_TSK_MAX_SUPPORT_NUM                          8
/* 缺省的任务栈大小 */
#define OS_TSK_DEFAULT_STACK_SIZE                       0x800
/* IDLE任务栈的大小 */
#define OS_TSK_IDLE_STACK_SIZE                          0x800
/* 任务栈初始化魔术字，默认是0xCA，只支持配置一个字节 */
#define OS_TSK_STACK_MAGIC_WORD                         0xCA

/* ***************************** 配置CPU占用率及CPU告警模块 **************** */
/* CPU占用率模块裁剪开关 */
#define OS_INCLUDE_CPUP                                 YES
/* 采样时间间隔(单位tick)，若其值大于0，则作为采样周期，否则两次调用PRT_CpupNow或PRT_CpupThread间隔作为周期 */
#define OS_CPUP_SAMPLE_INTERVAL                         10
/* CPU占用率告警动态配置项 */
#define OS_CONFIG_CPUP_WARN                             YES
/* CPU占用率告警阈值(精度为万分比) */
#define OS_CPUP_SHORT_WARN                              8500
/* CPU占用率告警恢复阈值(精度为万分比) */
#define OS_CPUP_SHORT_RESUME                            7500

/* ***************************** 配置内存管理模块 ************************** */
/* 用户可以创建的最大分区数，取值范围[0,253] */
#define OS_MEM_MAX_PT_NUM                               10
/* 私有FSC内存分区起始地址 */
#define OS_MEM_FSC_PT_ADDR                              (uintptr_t)(rmem_start)
/* 私有FSC内存分区大小 */
#define OS_MEM_FSC_PT_SIZE                              0x3000000

/* ***************************** 配置信号量管理模块 ************************* */
/* 信号量模块裁剪开关 */
#define OS_INCLUDE_SEM                                  YES

/* 最大支持的信号量数 */
#define OS_SEM_MAX_SUPPORT_NUM                          10

/* ***************************** 配置队列模块 ******************************* */
/* 队列模块裁剪开关 */
#define OS_INCLUDE_QUEUE                                YES
/* 最大支持的队列数,范围(0,0xFFFF] */
#define OS_QUEUE_MAX_SUPPORT_NUM                        10

/* ************************* 钩子模块配置 *********************************** */
/* 硬中断进入钩子最大支持个数, 范围[0, 255] */
#define OS_HOOK_HWI_ENTRY_NUM                           10
/* 硬中断退出钩子最大支持个数, 范围[0, 255] */
#define OS_HOOK_HWI_EXIT_NUM                            10
/* 任务切换钩子最大支持个数, 范围[0, 255] */
#define OS_HOOK_TSK_SWITCH_NUM                          3
/* IDLE钩子最大支持个数, 范围[0, 255] */
#define OS_HOOK_IDLE_NUM                                4

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_CONFIG_H */
