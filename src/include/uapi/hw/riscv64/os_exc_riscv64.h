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
 * Description: 异常模块的对外头文件。
 */
#ifndef RISCV64_EXC_H
#define RISCV64_EXC_H

#include "prt_typedef.h"
#include "prt_sys.h"
#include "prt_task.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */


#define HWEXC_IAM		0   //硬件异常_指令地址未对齐
#define HWEXC_IAF		1   //硬件异常_指令获取错误 [没有对应的读指令权限]
#define HWEXC_InI		2   //硬件异常_指令二进制码无法识别
#define HWEXC_Break		3   //硬件异常_断点调试异常Break_Point 
#define HWEXC_LAM		4   //硬件异常_load指令地址未对齐 [load address misaligned]
#define HWEXC_LAF		5   //硬件异常_load指令获取错误 [没有对应的读数据权限]
#define HWEXC_SAAM		6   //硬件异常_Store/AMO address misaligned
#define HWEXC_SAAF		7   //硬件异常_Store/AMO address fault
#define HWEXC_ECALLU	8   //硬件陷入_从U态转换的ECALL陷入
#define HWEXC_ECALLS	9   //硬件陷入_从S态转化的ECALL陷入
#define HWEXC_Reserve_1 10  //硬件陷入_保留字
#define HWEXC_ECALLM   	11  //硬件陷入_从M态转化的ECALL陷入
#define HWEXC_IPG		12  //硬件异常_Instruction Page Fault 	没有MMU的执行权限导致的PageFault 
#define HWEXC_LPG		13  //硬件异常_Load Page Fault 		没有MMU的读权限导致的PageFault
#define HWEXC_Reserve_2	14  //硬件异常_保留字
#define HWEXC_SAPGF		15  //硬件异常_Store/AMO page fault

struct ExcCalleeInfo {
	U64 sp;
	U64 s0;
	U64 fp;
	U64 s2;
	U64 s3;
	U64 s4;
	U64 s5;
	U64 s6;
	U64 s7;
	U64 s8;
	U64 s9;
	U64 s10;
	U64 s11;
}; //callee register context

struct ExcCauseRegInfo {
	U64 mcause;		//异常原因
	U64 mepc;		//异常发生地址
	U64 mstatus;	//异常发生时的状态寄存器
	U64 mtval; 		//辅助一些异常如 load page fault 发生时，想load的地址发生在mtval
};

struct ExcInfo {
	/* OS版本号 */
    char osVer[OS_SYS_OS_VER_LEN];
    /* 产品版本号 */
    char appVer[OS_SYS_APP_VER_LEN];
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
	/* 异常前栈底 */ 
    uintptr_t stackBottom;
	/* 异常发生前的任务信息*/
	struct TskInfo* task;
	/* 异常原因 */
    struct ExcCauseRegInfo excCause;
	/* 异常时上下文 */
	struct ExcCalleeInfo excContext;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* RISCV64_EXC_H */
