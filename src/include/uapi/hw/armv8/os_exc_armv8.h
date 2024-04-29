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
 * Description: 异常模块的对外头文件。
 */
#ifndef ARMV8_EXC_H
#define ARMV8_EXC_H

#include "prt_typedef.h"
#include "prt_sys.h"
#if defined(OS_OPTION_HAVE_FPU)
#include "os_cpu_armv8.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * OS使用的EL个数，EL1~EL3
 */
#define OS_ELX_NUM      3

/*
 * 通用寄存器个数, x0~x30
 */
#define XREGS_NUM       31

/*
 * 异常寄存器信息结构
 */
struct ExcElxRegs {
    uintptr_t esr;
    uintptr_t far;
    uintptr_t spsr;
    uintptr_t elr;
    uintptr_t sctlr;
    uintptr_t sp;
    uintptr_t vbar;
    uintptr_t ttbr0;
    uintptr_t tcr;
    uintptr_t mair;
};

struct ExcRegInfo {
    uintptr_t ttbr0;
    uintptr_t ttbr1;
    uintptr_t tcr;
    uintptr_t mair;
    uintptr_t sctlr;
    uintptr_t vbar;
    uintptr_t currentEl;
    uintptr_t sp;
    // 以下字段的内存布局与TskContext保持一致
#if defined(OS_OPTION_HAVE_FPU)
    __uint128_t q[OS_FPU_CONTEXT_REG_NUM];
    uintptr_t fpcr;
    uintptr_t fpsr;
#endif
    uintptr_t elr;                  // 返回地址
    uintptr_t spsr;
    uintptr_t far;
    uintptr_t esr;
    uintptr_t xzr;
    uintptr_t xregs[XREGS_NUM];     // 0~30 : x30~x0
};

/*
 * CpuTick结构体类型。
 *
 * 用于记录64位的cycle计数值。
 */
struct SreCpuTick {
    U32 cntHi; /* cycle计数高32位 */
    U32 cntLo; /* cycle计数低32位 */
};

/*
 * 异常信息结构体
 */
struct ExcInfo {
    // OS版本号
    char osVer[OS_SYS_OS_VER_LEN];
    // 产品版本号
    char appVer[OS_SYS_APP_VER_LEN];
    // 异常原因
    U32 excCause;
    // 异常前的线程类型
    U32 threadType;
    // 异常前的线程ID, 该ID组成threadID = LTID
    U32 threadId;
    // 字节序
    U16 byteOrder;
    // CPU类型
    U16 cpuType;
    // CPU ID
    U32 coreId;
    // CPU Tick
    struct SreCpuTick cpuTick;
    // 异常嵌套计数
    U32 nestCnt;
    // 致命错误码，发生致命错误时有效
    U32 fatalErrNo;
    // 异常前栈指针
    uintptr_t sp;
    // 异常前栈底
    uintptr_t stackBottom;
    // 异常发生时的核内寄存器上下文信息
    struct ExcRegInfo regInfo;
};

/*
 * ARMV8异常具体类型:异常原因参见ESR寄存器。
 */
#define OS_EXCEPT_ESR           0

/*
 * ARMV8异常具体类型:其他核异常。
 */
#define OS_EXCEPT_OTHER_CORE    1

/*
 * ARMV8异常具体类型:致命错误异常。
 */
#define OS_EXCEPT_FATALERROR    2

/*
 * ARMV8异常具体类型:栈越界异常。
 */
#define OS_EXCEPT_STACKOVERFLOW 3

/*
 * ARMV8异常具体类型:非法指令异常。
 */
#define OS_EXCEPT_UNDEF_INSTR   4

/*
 * ARMV8异常具体类型:数据中止异常。
 */
#define OS_EXCEPT_DATA_ABORT    5

/*
 * ARMV8异常具体类型:快速中断异常。
 */
#define OS_EXCEPT_FIQ           6

/*
 * ARMV8异常具体类型:pc非对齐异常。
 */
#define OS_EXCEPT_PC_NOT_ALIGN  7

/*
 * ARMV8异常具体类型:sp非对齐异常。
 */
#define OS_EXCEPT_SP_NOT_ALIGN  8

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* ARMV8_EXC_H */
