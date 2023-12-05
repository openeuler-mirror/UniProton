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
#include "prt_hwi.h"

struct TagOsStack {
    U64 rax;
    U64 rbx;
    U64 rcx;
    U64 rdx;
    U64 rsi;
    U64 rdi;
    U64 rbp;
    U64 r8;
    U64 r9;
    U64 r10;
    U64 r11;
    U64 r12;
    U64 r13;
    U64 r14;
    U64 r15;
    U64 intNumber;
    U64 rbpFrame;
    U64 ripFrame;
    U64 error;
    U64 rip;
    U64 cs;
    U64 rflags;
    U64 rsp;
    U64 ss;
};

struct OsFpuStack {
    U16 fcw;
    U16 fsw;
    U8  ftw;
    U8  reserve1;
    U16 fop;
    U32 fip;
    U16 fcs;
    U16 reserve2;
    U32 fdp;
    U16 fds;
    U16 reserve3;
    U32 mxcsr;
    U32 mxcsrMask;
    // fxsave/fsrstore 512 字节，后面栈初始化为0
};

#define OS_FPU_SIZE 512
#define OS_CONTEXT_EMPTY_SAPCE (OS_FPU_SIZE - sizeof(struct OsFpuStack))

struct TskContext {
    struct OsFpuStack fpuStack;
    char resv[OS_CONTEXT_EMPTY_SAPCE];
    struct TagOsStack tagStack;
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
#define OsIntEnable()     PRT_HwiLock()
#define OsIntDisable()    PRT_HwiUnLock()

#endif /* OS_CPU_X86_64 */
