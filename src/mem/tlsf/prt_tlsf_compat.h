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
 * Description: TLSF 类型/宏兼容层。把算法源码依赖的类型与编译器宏映射到 UniProton
 *              的 prt_typedef.h，并保证 UINTPTR 在 64 位平台为 8 字节。同时合并了原
 *              debug/hook/task 三个 shim 的内容。本文件为移植适配层，不改动算法流程。
 */
#ifndef _PRT_TLSF_COMPAT_H
#define _PRT_TLSF_COMPAT_H

#include "prt_typedef.h"
#include "prt_task.h"
#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/* =====================================================================
 * 基本整型别名：映射到 UniProton 类型（prt_typedef.h 已定义 U8/U16/U32/U64/S*）
 * ===================================================================== */

#ifndef UINT8
typedef U8 UINT8;
#endif

#ifndef UINT16
typedef U16 UINT16;
#endif

#ifndef UINT32
typedef U32 UINT32;
#endif

#ifndef UINT64
typedef U64 UINT64;
#endif

#ifndef INT8
typedef S8 INT8;
#endif

#ifndef INT16
typedef S16 INT16;
#endif

#ifndef INT32
typedef S32 INT32;
#endif

#ifndef INT64
typedef S64 INT64;
#endif

#ifndef CHAR
typedef char CHAR;
#endif

/* =====================================================================
 * 指针宽度类型：UINTPTR 必须与目标平台指针宽度一致。LiteOS 原始定义为
 * unsigned int(4字节)，在 aarch64 上会截断指针导致内存算法出错，这里用
 * uintptr_t 修正。
 * ===================================================================== */

#ifndef UINTPTR
typedef uintptr_t UINTPTR;
#endif

#ifndef INTPTR
typedef intptr_t INTPTR;
#endif

/* =====================================================================
 * 基础类型/关键字别名
 * ===================================================================== */

#ifndef BOOL
typedef bool BOOL;
#endif

#ifndef VOID
#define VOID void
#endif

#ifndef STATIC
#define STATIC static
#endif

/* INLINE：prt_typedef.h 把 INLINE 定义为 "static __inline ..."，会使算法里的
   "STATIC INLINE" 展开成 "static static __inline" 触发 duplicate static 错误。
   此处覆盖为不含 static 的版本，使 "STATIC INLINE" == "static inline"。 */
#ifdef INLINE
#undef INLINE
#endif
#define INLINE inline __attribute__((always_inline))

#ifndef STATIC_INLINE
#define STATIC_INLINE static inline
#endif

/* =====================================================================
 * 段属性宏：移植到 UniProton 时统一置空（对象仍正常进入 .a，段布局由 UniProton 控制）。
 * 各宏 #ifndef 守护，若 UniProton 已定义则沿用其定义。
 * ===================================================================== */

#ifndef PRT_TLSF_SEC_ALW_INLINE
#define PRT_TLSF_SEC_ALW_INLINE
#endif

#ifndef PRT_TLSF_SEC_TEXT
#define PRT_TLSF_SEC_TEXT
#endif

#ifndef PRT_TLSF_SEC_TEXT_MINOR
#define PRT_TLSF_SEC_TEXT_MINOR
#endif

#ifndef PRT_TLSF_SEC_TEXT_INIT
#define PRT_TLSF_SEC_TEXT_INIT
#endif

#ifndef PRT_TLSF_SEC_DATA
#define PRT_TLSF_SEC_DATA
#endif

#ifndef PRT_TLSF_SEC_DATA_INIT
#define PRT_TLSF_SEC_DATA_INIT
#endif

#ifndef PRT_TLSF_SEC_BSS
#define PRT_TLSF_SEC_BSS
#endif

#ifndef PRT_TLSF_SEC_BSS_MINOR
#define PRT_TLSF_SEC_BSS_MINOR
#endif

#ifndef PRT_TLSF_SEC_BSS_INIT
#define PRT_TLSF_SEC_BSS_INIT
#endif

#ifndef PRT_TLSF_SEC_TEXT_DATA
#define PRT_TLSF_SEC_TEXT_DATA
#endif

#ifndef PRT_TLSF_SEC_TEXT_BSS
#define PRT_TLSF_SEC_TEXT_BSS
#endif

#ifndef PRT_TLSF_SEC_TEXT_RODATA
#define PRT_TLSF_SEC_TEXT_RODATA
#endif

#ifndef PRT_TLSF_SEC_SYMDATA
#define PRT_TLSF_SEC_SYMDATA
#endif

#ifndef PRT_TLSF_SEC_SYMBSS
#define PRT_TLSF_SEC_SYMBSS
#endif

#ifndef PRT_TLSF_SEC_VEC
#define PRT_TLSF_SEC_VEC
#endif

#ifndef PRT_TLSF_SEC_KEEP_DATA_DDR
#define PRT_TLSF_SEC_KEEP_DATA_DDR
#endif

#ifndef PRT_TLSF_SEC_KEEP_TEXT_DDR
#define PRT_TLSF_SEC_KEEP_TEXT_DDR
#endif

#ifndef PRT_TLSF_SEC_KEEP_DATA_SRAM
#define PRT_TLSF_SEC_KEEP_DATA_SRAM
#endif

#ifndef PRT_TLSF_SEC_KEEP_TEXT_SRAM
#define PRT_TLSF_SEC_KEEP_TEXT_SRAM
#endif

/* =====================================================================
 * 编译器属性宏
 * ===================================================================== */

/* CLZ：GCC 内建，aarch64/x86_64/riscv64 均支持 __builtin_clz */
#ifndef CLZ
#define CLZ __builtin_clz
#endif

#ifndef ASM
#define ASM __asm
#endif

#ifndef WEAK
#define WEAK __attribute__((weak))
#endif

#ifndef USED
#define USED __attribute__((used))
#endif

#ifndef NORETURN
#define NORETURN __attribute__((__noreturn__))
#endif

#ifndef UNREACHABLE
#define UNREACHABLE __builtin_unreachable()
#endif

/* =====================================================================
 * 返回码 / 特殊值（prt_typedef.h 已定义 OS_OK/OS_ERROR 时沿用）
 * ===================================================================== */

#ifndef OS_OK
#define OS_OK 0U
#endif

#ifndef OS_ERROR
#define OS_ERROR (UINT32)(-1)
#endif

#ifndef OS_NULL_BYTE
#define OS_NULL_BYTE ((UINT8)0xFF)
#endif

#ifndef OS_NULL_SHORT
#define OS_NULL_SHORT ((UINT16)0xFFFF)
#endif

#ifndef OS_NULL_INT
#define OS_NULL_INT ((UINT32)0xFFFFFFFF)
#endif

/* 原子类型（算法内部使用） */
typedef volatile INT32 Atomic;
typedef volatile INT64 Atomic64;

/* =====================================================================
 * 字节序
 * ===================================================================== */

#ifndef OS_LITTLE_ENDIAN
#define OS_LITTLE_ENDIAN 0x1234
#endif

#ifndef OS_BIG_ENDIAN
#define OS_BIG_ENDIAN 0x4321
#endif

#ifndef OS_BYTE_ORDER
#define OS_BYTE_ORDER OS_LITTLE_ENDIAN
#endif

#ifndef UNUSED
#define UNUSED(X) (void)X
#endif

/* =====================================================================
 * 杂项工具宏 / 内联
 * ===================================================================== */

/* OsTlsfAlign：仅在 bitmap 扫描处用于 32 位 index 对齐，输入为小整数 */
static inline UINT32 OsTlsfAlign(UINT32 addr, UINT32 boundary)
{
    return (addr + (boundary - 1)) & ~(boundary - 1);
}

#define OS_GOTO_ERREND() do { goto OS_TLSF_ERREND; } while (0)
#define OS_TLSF_ASSERT_COND(expression)

#ifndef asm
#define asm __asm
#endif

#ifdef typeof
#undef typeof
#endif
#define typeof __typeof__

#define ALIAS_OF(of) __attribute__((alias(#of)))
#define FUNC_ALIAS(real_func, new_alias, args_list, return_type) \
    return_type new_alias args_list ALIAS_OF(real_func)

/* =====================================================================
 * 以下三组（打印 / hook / 任务适配）合并入本兼容层，避免散落小文件。
 * ===================================================================== */

/* 调试打印：PRT_Printf 由各板级 BSP 提供，直驱 UART，启动早期即可用。 */
extern unsigned int PRT_Printf(const char *format, ...);

#define PRINTK(fmt, ...)        (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_RELEASE(fmt, ...) (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_INFO(fmt, ...)    (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_WARN(fmt, ...)    (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_ERR(fmt, ...)     (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_DEBUG(fmt, ...)   (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_EXC(fmt, ...)     (void)PRT_Printf(fmt, ##__VA_ARGS__)

#define OS_TLSF_Panic(fmt, ...) do { \
    PRT_Printf(fmt, ##__VA_ARGS__); \
    while (1) { } \
} while (0)

/* 内存算法 hook：UniProton 有自己的 hook 子系统，此处置空操作，不影响算法主流程。 */
typedef enum {
    OS_TLSF_HOOK_TYPE_MEM_INIT = 0,
    OS_TLSF_HOOK_TYPE_MEM_DEINIT,
    OS_TLSF_HOOK_TYPE_MEM_ALLOC,
    OS_TLSF_HOOK_TYPE_MEM_ALLOCALIGN,
    OS_TLSF_HOOK_TYPE_MEM_FREE,
    OS_TLSF_HOOK_TYPE_MEM_REALLOC
} OS_TLSF_HOOK_TYPE;

#define OsHookCall(hook, ...) (void)0

/* 任务接口适配：TLSF_CFG_TASK_MEM_USED 开启时，节点头记录申请任务的 handle。 */
#define OS_TASK_ERRORID 0xFFFFFFFFU
#define OS_TASK_STATUS_UNUSED OS_TSK_UNUSED

struct OsTlsfTaskCb {
    UINT32 taskStatus;
    VOID (*taskEntry)(void);
    UINT32 taskID;
    CHAR *taskName;
};
typedef struct OsTlsfTaskCb OsTlsfTaskCb;

static inline UINT32 OsTlsfCurTaskIDGet(VOID)
{
    TskHandle taskPid;
    UINT32 taskId;

    if (PRT_TaskSelf(&taskPid) != OS_OK) {
        return OS_TASK_ERRORID;
    }

    taskId = GET_HANDLE(taskPid);
    return (taskId < TLSF_CFG_BASE_CORE_TSK_LIMIT) ? taskId : OS_TASK_ERRORID;
}

static inline OsTlsfTaskCb *OsTlsfTcbFromTid(UINT32 tid)
{
    static OsTlsfTaskCb taskCb;
    static CHAR taskName[OS_TSK_NAME_LEN];
    struct TskInfo taskInfo = {0};
    TskHandle taskPid = COMPOSE_PID(OS_THIS_CORE, tid);
    UINT32 i;

    taskCb.taskStatus = OS_TASK_STATUS_UNUSED;
    taskCb.taskEntry = NULL;
    taskCb.taskID = tid;
    taskCb.taskName = NULL;

    if ((tid >= TLSF_CFG_BASE_CORE_TSK_LIMIT) || (PRT_TaskGetInfo(taskPid, &taskInfo) != OS_OK)) {
        return &taskCb;
    }

    for (i = 0; i < (OS_TSK_NAME_LEN - 1); i++) {
        taskName[i] = taskInfo.name[i];
        if (taskInfo.name[i] == '\0') {
            break;
        }
    }
    taskName[OS_TSK_NAME_LEN - 1] = '\0';

    taskCb.taskStatus = taskInfo.taskStatus;
    taskCb.taskEntry = taskInfo.entry;
    taskCb.taskID = tid;
    taskCb.taskName = taskName;
    return &taskCb;
}

#define OS_TLSF_TCB_FROM_TID(tid) OsTlsfTcbFromTid(tid)

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */
#endif /* _PRT_TLSF_COMPAT_H */
