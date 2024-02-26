/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-01-09
 * Description: RISCV64 任务模块的对外头文件。
 */
#ifndef RISCV64_M_TASK_H
#define RISCV64_M_TASK_H

#include "prt_typedef.h"
#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 任务上下文的结构体定义。
 */
struct TskContext {
#if defined(OS_ARCH_SURPORT_F)
    U64 fcsr;
    U64 fs11;
    U64 fs10;
    U64 fs9;
    U64 fs8;
    U64 fs7;
    U64 fs6;
    U64 fs5;
    U64 fs4;
    U64 fs3;
    U64 fs2;
    U64 fs1;
    U64 fs0;
#endif
    //普通寄存器
    U64 mepc;
    U64 mstatus;
    U64 t6;
    U64 t5;
    U64 t4;
    U64 t3;
    U64 s11;
    U64 s10;
    U64 s9;
    U64 s8;
    U64 s7;
    U64 s6;
    U64 s5;
    U64 s4;
    U64 s3;
    U64 s2;
    U64 a7;
    U64 a6;
    U64 a5;
    U64 a4;
    U64 a3;
    U64 a2;
    U64 a1;
    U64 a0;
    U64 s1;
    U64 s0;
    U64 t2;
    U64 t1;
    U64 t0;
    U64 tp;
    U64 gp;
    U64 ra;
};

/*
 *  描述: 读取当前核号
 */
OS_SEC_ALW_INLINE INLINE U32 OsGetCoreID(void)
{
    U64 mpid;
    OS_EMBED_ASM("csrr %0, mhartid":"=r" (mpid)::);
    return mpid;
}

/*
 * 获取当前核ID
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_GetCoreID(void)
{
    return OsGetCoreID();
}

#define PRT_DSB() OS_EMBED_ASM("" : : : "memory")
#define PRT_DMB() OS_EMBED_ASM("" : : : "memory")
#define PRT_ISB() OS_EMBED_ASM("" : : : "memory")


#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* RISCV64_M_TASK_H */
