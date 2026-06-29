/*
 * Copyright (c) 2009-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-06-01
 * Description: TLSF 内存算法配置宏。为 prt_tlsf_core.c 提供 TLSF_CFG_* 安全默认值。
 *              各宏的「能否开启 / 功能 / 依赖」说明详见 doc/design/mem.md「TLSF 配置宏」。
 *              本文件只提供宏配置，不改动算法源码流程。
 */
#ifndef _PRT_TLSF_CONFIG_H
#define _PRT_TLSF_CONFIG_H

#include "prt_buildef.h"
#include "prt_config.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* =====================================================================
 * 系统堆：移植层使用外部传入的分区（UniProton 的 OS_MEM_FSC_PT_ADDR/SIZE），
 * 不使用算法自带的静态堆数组；OsMemSystemInit 不会被调用，以下宏仅占位。
 * ===================================================================== */

/* 使用外部堆（由 UniProton 传入分区地址），不使用自带静态堆 */
#ifndef TLSF_CFG_SYS_EXTERNAL_HEAP
#define TLSF_CFG_SYS_EXTERNAL_HEAP 1
#endif

/* 自带静态堆起始地址（仅占位，实际不使用） */
#ifndef TLSF_CFG_SYS_HEAP_ADDR
#define TLSF_CFG_SYS_HEAP_ADDR ((void *)0)
#endif

/* 自带静态堆大小（仅占位，实际不使用） */
#ifndef TLSF_CFG_SYS_HEAP_SIZE
#define TLSF_CFG_SYS_HEAP_SIZE 0x1000
#endif

/* =====================================================================
 * 内存特性裁剪
 * ===================================================================== */

/* 内存使用水位（峰值）记录，供统计查询。自包含，可开启 */
#ifndef TLSF_CFG_MEM_WATERLINE
#define TLSF_CFG_MEM_WATERLINE 1
#endif

/* 多内存池支持。自包含，可开启 */
#ifndef TLSF_CFG_MEM_MUL_POOL
#define TLSF_CFG_MEM_MUL_POOL 1
#endif

/* 多段不连续内存加入同一池（OsTlsfRegionsAdd）。自包含，可开启 */
#ifndef TLSF_CFG_MEM_MUL_REGIONS
#define TLSF_CFG_MEM_MUL_REGIONS 0
#endif

/* 节点完整性检查（魔字 / 链表一致性）。task 查询已对接 UniProton PRT_TaskGetInfo */
#ifndef TLSF_CFG_BASE_MEM_NODE_INTEGRITY_CHECK
#define TLSF_CFG_BASE_MEM_NODE_INTEGRITY_CHECK 0
#endif

/* 内存泄漏检查（记录调用栈）。依赖 OsBackTraceHookCall，未移植，勿开启 */
#ifndef TLSF_CFG_MEM_LEAKCHECK
#define TLSF_CFG_MEM_LEAKCHECK 0
#endif

/* 节点头记录申请该块的任务 ID。影响节点头布局；64 位平台必须为 1 */
#ifndef TLSF_CFG_TASK_MEM_USED
#define TLSF_CFG_TASK_MEM_USED 1
#endif

/* 按任务 ID 释放内存。TASK_MEM_USED!=1 且 TSK_LIMIT+1>64 时不支持 */
#ifndef TLSF_CFG_MEM_FREE_BY_TASKID
#define TLSF_CFG_MEM_FREE_BY_TASKID 0
#endif

/* 泄漏检查记录的返回地址层数（依赖 LEAKCHECK，未移植） */
#ifndef TLSF_CFG_MEM_RECORD_LR_CNT
#define TLSF_CFG_MEM_RECORD_LR_CNT 0
#endif

/* 泄漏检查跳过的栈帧数（依赖 LEAKCHECK，未移植） */
#ifndef TLSF_CFG_MEM_OMIT_LR_CNT
#define TLSF_CFG_MEM_OMIT_LR_CNT 0
#endif

/* 泄漏检查记录条数上限（依赖 LEAKCHECK，未移植） */
#ifndef TLSF_CFG_MEM_LEAKCHECK_RECORD_MAX_NUM
#define TLSF_CFG_MEM_LEAKCHECK_RECORD_MAX_NUM 0
#endif

/* 任务数上限，用于 taskID 相关校验 */
#ifndef TLSF_CFG_BASE_CORE_TSK_LIMIT
#ifdef OS_TSK_MAX_SUPPORT_NUM
#define TLSF_CFG_BASE_CORE_TSK_LIMIT OS_TSK_MAX_SUPPORT_NUM
#else
#define TLSF_CFG_BASE_CORE_TSK_LIMIT 31
#endif
#endif

/* TLSF 内部打印开关 */
#ifndef TLSF_CFG_KERNEL_PRINTF
#define TLSF_CFG_KERNEL_PRINTF 1
#endif

/* 与异常子系统联动。依赖额外异常 dump 模块，未适配，勿开启 */
#ifndef TLSF_CFG_PLATFORM_EXC
#define TLSF_CFG_PLATFORM_EXC 0
#endif

/* =====================================================================
 * 以下特性依赖未移植的子系统，必须保持关闭
 * ===================================================================== */

/* LMK：内存耗尽时按策略 kill 任务回收内存。依赖 OsTlsfLmkTasksKill，未移植 */
#define TLSF_CFG_KERNEL_LMK 0

/* LMS：内存越界 / use-after-free 检测。依赖 LMS 模块，未适配。
   prt_tlsf_core.c 中以 #ifdef 判断，故此处刻意不定义以保持关闭。 */
/* TLSF_CFG_KERNEL_LMS （不定义） */

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _PRT_TLSF_CONFIG_H */
