/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-25
 * Description: cpu架构相关的外部头文件
 */
#ifndef OS_CPU_X86_64
#define OS_CPU_X86_64

#include "prt_typedef.h"

/*
 * 任务上下文的结构体定义。
 */
struct TskContext {
    /* 当前物理寄存器R0-R12 */
    U32 r[13];
    /* 程序计数器 */
    U32 pc;
    /* 即R14链接寄存器 */
    U32 lr;
    /* 当前程序状态寄存器 */
    U32 cpsr;
};

OS_SEC_ALW_INLINE INLINE U32 OsGetCoreID(void)
{
    return 0;
}

/*
 * 获取当前核ID
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_GetCoreID(void)
{
    return OsGetCoreID();
}

#define DIV64(a, b) ((a) / (b))
#define DIV64_REMAIN(a, b) ((a) % (b))

#endif /* OS_CPU_X86_64 */
