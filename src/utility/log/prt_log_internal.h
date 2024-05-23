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
 * Create: 2024-03-19
 * Description: log模块的模块内头文件。
 */

#ifndef PRT_LOG_INTERNAL_H
#define PRT_LOG_INTERNAL_H

#include "prt_log.h"

#define SHM_USED_SIZE 0x1004200UL // ~16MB
#define VALID_FLAGS_OFFSET 0x1000000UL
#define VALID_FLAGS_SIZE 0x4000UL
#define HEAD_PTR_OFFSET 0x1004000UL
#define TAIL_PTR_OFFSET 0x1004100UL

#define BUFFER_BLOCK_NUM 0x4000UL
#define BUFFER_BLOCK_SIZE 0x400UL /* 1KB */

struct logHeader {
    U64 sec;
    U64 nanoSec;
    U64 sequenceNum;
    U32 taskPid;
    U16 len;
    U8 facility;
    U8 level;
    U8 logContent[];
};

#if defined(OS_ARCH_ARMV7_M)
    #define STORE_FENCE() __asm__ __volatile__ ("dmb sy" : : : "memory");
    #define M_FENCE() __asm__ __volatile__ ("dmb sy" : : : "memory");
    #define LOAD_FENCE() __asm__ __volatile__ ("dmb sy" : : : "memory"); 
#endif

#if defined(OS_ARCH_ARMV8)
    #define STORE_FENCE() __asm__ __volatile__ ("dmb st" : : : "memory");
    #define M_FENCE() __asm__ __volatile__ ("dmb sy" : : : "memory");
    #define LOAD_FENCE() __asm__ __volatile__ ("dmb ld" : : : "memory");
#endif

#if defined(OS_ARCH_X86_64)
    #define STORE_FENCE() __asm__ __volatile__ ("sfence" : : : "memory");
    #define M_FENCE() __asm__ __volatile__ ("mfence" : : : "memory");
    #define LOAD_FENCE() __asm__ __volatile__ ("lfence" : : : "memory");
#endif

#if defined(OS_ARCH_RISCV64G)
    #define STORE_FENCE() __asm__ __volatile__ ("fence w,w" : : : "memory");
    #define M_FENCE() __asm__ __volatile__ ("fence rw,rw" : : : "memory");
    #define LOAD_FENCE() __asm__ __volatile__ ("fence r,r" : : : "memory");
#endif
#define LOG_MAX_SIZE (BUFFER_BLOCK_SIZE - sizeof(struct logHeader))

#endif /* PRT_LOG_INTERNAL_H */