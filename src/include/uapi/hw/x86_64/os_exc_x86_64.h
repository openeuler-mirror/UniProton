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
 * Description: 异常模块的对外头文件。
 */
#ifndef OS_EXC_X86_64_H
#define OS_EXC_X86_64_H

#include "prt_typedef.h"
#include "prt_sys.h"

/*
 * 异常寄存器信息结构
 */
struct ExcRegInfo {
    U32 usp;        /* 发生异常前的USP寄存器值 */
    U32 sp;         /* 发生异常前的SSP寄存器值 */
    U32 isr;        /* 发生异常后的异常相关ISR寄存器值 */
    U32 emaddr;     /* 发生异常后的EMADDR寄存器值，导致异常的地址 */
    U32 lbeg0;
    U32 lend0;
    U32 lcount0;
    U32 prex1;
    U32 lbeg1;
    U32 lend1;
    U32 lcount1;
    U32 cidr;
    U32 a[32];
};

/*
 * CpuTick结构体类型，用于记录64位的cycle计数值。
 */
struct PrtCpuTick {
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
    // 致命错误码
    U32 fatalErrNo;
    // 异常前的线程类型
    U32 threadType;
    // 异常前的线程ID
    U32 threadID;
    // 字节序
    U16 byteOrder;
    // CPU类型
    U16 cpuType;
    // CPU ID
    U32 coreId;
    // CPU Tick
    struct PrtCpuTick cpuTick;
    // 异常嵌套计数
    U32 nestCnt;
    // 异常前栈指针
    U32 sp;
    // 异常前栈底
    U32 stackBottom;
    /* 异常发生时的核内寄存器上下文信息, 82\57必须位于152字节处，
     * 若有改动，需更新prt_asm_cpu_external.h中的OS_EXC_REGINFO_OFFSET宏
     */
    struct ExcRegInfo regInfo;
};

/*
 * 非法指令异常
 */
#define OS_EXCEPT_UNDEF_INSTR      1

/*
 * 软件触发异常
 */
#define OS_EXCEPT_SWI              2

/*
 * 预取指令异常
 */
#define OS_EXCEPT_PREFETCH_ABORT   3

/*
 * 数据中止异常
 */
#define OS_EXCEPT_DATA_ABORT       4

/*
 * 快速中断异常
 */
#define OS_EXCEPT_FIQ              5

/*
 * 致命错误异常
 */
#define OS_EXCEPT_FATALERROR       6

/*
 * 栈越界异常
 */
#define OS_EXCEPT_STACKOVERFLOW    7

#endif /* OS_EXC_X86_64_H */
