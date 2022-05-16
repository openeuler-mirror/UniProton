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
 * Description: 异常模块的对外头文件。
 */
#ifndef ARMV7_M_EXC_H
#define ARMV7_M_EXC_H

#include "prt_typedef.h"
#include "prt_sys.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 寄存器信息结构
 *
 * 描述:MX平台下的异常触发时保存的寄存器信息。
 *
 * 注意:以下寄存器名称对应芯片手册中的寄存器名称。
 */
struct ExcContext {
    U32 r4; /* R4寄存器 */
    U32 r5; /* R5寄存器 */
    U32 r6; /* R6寄存器 */
    U32 r7; /* R7寄存器 */
    U32 r8; /* R8寄存器 */
    U32 r9; /* R9寄存器 */
    U32 r10; /* R10寄存器 */
    U32 r11; /* R11寄存器 */
    U32 basePri; /* 中断优先级屏蔽寄存器 */
    U32 sp; /* 程序栈指针 */
    U32 r0; /* RO寄存器 */
    U32 r1; /* R1寄存器 */
    U32 r2; /* R2寄存器 */
    U32 r3; /* R3寄存器 */
    U32 r12; /* R12寄存器 */
    U32 lr; /* 程序返回地址，即异常时的下一条指令 */
    U32 pc; /* 异常时的PC指针 */
    U32 xpsr; /* xpsr程序状态寄存器 */
};

/*
 * 异常信息结构体
 *
 * 描述:MX平台下的异常触发时保存的异常信息。
 *
 */
struct ExcRegInfo {
    /*
     * 上一次异常类型, bit16标示FaultAddr域是否有效，bit8~bit15表示当发生HardFault时的子异常错误类型，
     * bit0~bit7表示异常时的主异常错误类型
     */
    U32 excType;
    /* 若为精确地址访问错误表示异常发生时的错误访问地址,其它为缺省值0xABABABAB */
    U32 faultAddr;
    /* 异常发生时刻的硬件上下文 */
    struct ExcContext *context;
};

/*
 * 用户可以看到异常信息
 */
struct ExcInfo {
    /* OS版本号 */
    char osVer[OS_SYS_OS_VER_LEN];
    /* 产品版本号 */
    char appVer[OS_SYS_APP_VER_LEN];
    /* 异常原因 */
    U32 excCause;
    /* 异常前的线程类型 */
    U32 threadType;
    /* 异常前的线程ID */
    U32 threadId;
    /* 字节序 */
    U16 byteOrder;
    /* CPU类型 */
    U16 cpuType;
    /* CPU ID */
    U32 coreId;
    /* CPU Tick */
    U64 cpuTick;
    /* 异常嵌套计数 */
    U32 nestCnt;
    /* 异常前栈指针 */
    U32 sp;
    /* 异常前栈底 */
    U32 stackBottom;
    /*
     * 异常发生时的核内寄存器上下文信息，82\57必须位于152字节处，
     * 若有改动，需更新prt_asm_cpu_external.h中的OS_EXC_REGINFO_OFFSET宏
     */
    struct ExcRegInfo regInfo;
};

/*
 * Cortex-MX异常具体类型:总线状态寄存器入栈时发生错误。
 */
#define OS_EXC_BF_STKERR 1

/*
 * Cortex-MX异常具体类型:总线状态寄存器出栈时发生错误。
 */
#define OS_EXC_BF_UNSTKERR 2

/*
 * Cortex-MX异常具体类型:总线状态寄存器不精确的数据访问违例。
 */
#define OS_EXC_BF_IMPRECISERR 3

/*
 * Cortex-MX异常具体类型:总线状态寄存器精确的数据访问违例。
 */
#define OS_EXC_BF_PRECISERR 4

/*
 * Cortex-MX异常具体类型:总线状态寄存器取指时的访问违例。
 */
#define OS_EXC_BF_IBUSERR 5

/*
 * Cortex-MX异常具体类型:存储器管理状态寄存器入栈时发生错误。
 */
#define OS_EXC_MF_MSTKERR 6

/*
 * Cortex-MX异常具体类型:存储器管理状态寄存器出栈时发生错误。
 */
#define OS_EXC_MF_MUNSTKERR 7

/*
 * Cortex-MX异常具体类型:存储器管理状态寄存器数据访问违例。
 */
#define OS_EXC_MF_DACCVIOL 8

/*
 * Cortex-MX异常具体类型:存储器管理状态寄存器取指访问违例。
 */
#define OS_EXC_MF_IACCVIOL 9

/*
 * Cortex-MX异常具体类型:用法错误，表示除法运算时除数为零。
 */
#define OS_EXC_UF_DIVBYZERO 10

/*
 * Cortex-MX异常具体类型:用法错误，未对齐访问导致的错误。
 */
#define OS_EXC_UF_UNALIGNED 11

/*
 * Cortex-MX异常具体类型:用法错误，试图执行协处理器相关指令。
 */
#define OS_EXC_UF_NOCP 12

/*
 * Cortex-MX异常具体类型:用法错误，在异常返回时试图非法地加载EXC_RETURN到PC。
 */
#define OS_EXC_UF_INVPC 13

/*
 * Cortex-MX异常具体类型:用法错误，试图切入ARM状态。
 */
#define OS_EXC_UF_INVSTATE 14

/*
 * Cortex-MX异常具体类型:用法错误，执行的指令其编码是未定义的——解码不能。
 */
#define OS_EXC_UF_UNDEFINSTR 15

/*
 * Cortex-MX异常具体类型:NMI中断。
 */
#define OS_EXC_CAUSE_NMI 16

/*
 * Cortex-MX异常具体类型:硬fault。
 */
#define OS_EXC_CAUSE_HARDFAULT 17

/*
 * Cortex-MX异常具体类型:致命错误。
 */
#define OS_EXC_CAUSE_FATAL_ERR 18

/*
 * Cortex-MX异常具体类型:维测事件导致的硬fault。
 */
#define OS_EXC_CAUSE_DBGEVT 19

/*
 * Cortex-MX异常具体类型:取向量时发生的硬fault。
 */
#define OS_EXC_CAUSE_VECTBL 20

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* ARMV7_M_EXC_H */
