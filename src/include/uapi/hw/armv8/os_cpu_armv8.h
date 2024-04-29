/*
 * Copyright (c) 2022-2022 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2022-11-22
 * Description: cpu架构相关的外部头文件
 */
#ifndef OS_CPU_ARMV8_H
#define OS_CPU_ARMV8_H

#include "prt_buildef.h"
#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

// CurrentEl等级
#define CURRENT_EL_2       0x8
#define CURRENT_EL_1       0x4
#define CURRENT_EL_0       0x0

#define DAIF_DBG_BIT      (1U << 3)
#define DAIF_ABT_BIT      (1U << 2)
#define DAIF_IRQ_BIT      (1U << 1)
#define DAIF_FIQ_BIT      (1U << 0)

#define INT_MASK          (1U << 7)
#if (OS_MAX_CORE_NUM <= 4)
#define OS_CORE_ID_MASK          0xFFU
#define OS_CORE_MPID_CPUID(mpid) (((mpid) | ((mpid) >> 8)) & OS_CORE_ID_MASK)
#else
/* 22:21 2bit socket(32core/socket) ||  18:16 3bit clusterid (4core/cluster) || 9:8 2bit coreid */
#define OS_CORE_MPID_CPUID(mpid) ((((mpid) >> 21) & 0x3) << 5) + ((((mpid) >> 16) & 0x7) << 2) + (((mpid) >> 8) & 0x3)
// #define OS_CORE_MPID_CPUID(mpid) (((((mpid) >> 16) & 0xf) -8) * 4) +((mpid >> 8) & 0xf)
#endif

#define OS_FPU_CONTEXT_REG_NUM 32
/*
 * 任务上下文的结构体定义。
 */
struct TskContext {
#if defined(OS_OPTION_HAVE_FPU)
    __uint128_t q[OS_FPU_CONTEXT_REG_NUM];
    uintptr_t fpcr;
    uintptr_t fpsr;
#endif
    /* *< 当前物理寄存器R0-R12 */
    uintptr_t elr;               // 返回地址
    uintptr_t spsr;
    uintptr_t far;
    uintptr_t esr;
    uintptr_t xzr;
    uintptr_t x30;
    uintptr_t x29;
    uintptr_t x28;
    uintptr_t x27;
    uintptr_t x26;
    uintptr_t x25;
    uintptr_t x24;
    uintptr_t x23;
    uintptr_t x22;
    uintptr_t x21;
    uintptr_t x20;
    uintptr_t x19;
    uintptr_t x18;
    uintptr_t x17;
    uintptr_t x16;
    uintptr_t x15;
    uintptr_t x14;
    uintptr_t x13;
    uintptr_t x12;
    uintptr_t x11;
    uintptr_t x10;
    uintptr_t x09;
    uintptr_t x08;
    uintptr_t x07;
    uintptr_t x06;
    uintptr_t x05;
    uintptr_t x04;
    uintptr_t x03;
    uintptr_t x02;
    uintptr_t x01;
    uintptr_t x00;
};

/*
 *  描述: 读取当前核号
 *        使用mpidr 寄存器 (64bit) 根据核的线程模式获取核号
 *        bit 63-40 39-32   31  30 29~25  24 23-16  15-8   7~0
 *             res0  aff3  res1  u  res0  mt  aff2  aff1  aff0
 */
OS_SEC_ALW_INLINE INLINE U32 OsGetCoreID(void)
{
    U64 mpid;
    OS_EMBED_ASM("MRS  %0, MPIDR_EL1" : "=r"(mpid)::"memory", "cc");
    /* single-thread 模式下，核号取AFF0 AF1为0 */
    /* muti-thread 模式下，核号取AFF1 AF0为0 */
    /* 综上核号计算采用AFF0 + AFF1 */
    return OS_CORE_MPID_CPUID(mpid);
}

/*
 * 获取当前核ID
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_GetCoreID(void)
{
    return OsGetCoreID();
}

/* 开中断 */
OS_SEC_ALW_INLINE INLINE uintptr_t PRT_IntUnlock(void)
{
    uintptr_t state = 0;
    OS_EMBED_ASM(
        "mrs %0, DAIF           \n"
        "msr DAIFClr, %1        \n"
        : "=r"(state)
        : "i"(DAIF_IRQ_BIT | DAIF_FIQ_BIT)
        : "memory", "cc");
    return state & INT_MASK;
}
/* 关中断 */
OS_SEC_ALW_INLINE INLINE uintptr_t PRT_IntLock(void)
{
    uintptr_t state = 0;
    OS_EMBED_ASM(
        "mrs %0, DAIF           \n"
        "msr DAIFSet, %1        \n"
        : "=r"(state)
        : "i"(DAIF_IRQ_BIT | DAIF_FIQ_BIT)
        : "memory", "cc");
    return state & INT_MASK;
}
/* 恢复中断 */
OS_SEC_ALW_INLINE INLINE void PRT_IntRestore(uintptr_t intSave)
{
    if((intSave & INT_MASK) == 0) {
        OS_EMBED_ASM(
            "msr DAIFClr, %0        \n"
            :
            : "i"(DAIF_IRQ_BIT | DAIF_FIQ_BIT)
            : "memory", "cc");
    } else {
        OS_EMBED_ASM(
            "msr DAIFSet, %0        \n"
            :
            : "i"(DAIF_IRQ_BIT | DAIF_FIQ_BIT)
            : "memory", "cc");
    }
    return;
}

#define PRT_DSB() OS_EMBED_ASM("DSB sy" : : : "memory")
#define PRT_DMB() OS_EMBED_ASM("DMB sy" : : : "memory")
#define PRT_ISB() OS_EMBED_ASM("ISB" : : : "memory")

#define PRT_MemWait PRT_DSB

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* OS_CPU_ARMV8_H */
