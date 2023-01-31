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
 * Create: 2009-12-22
 * Description: mem模块的模块内头文件。
 */
#ifndef PRT_MEM_INTERNAL_H
#define PRT_MEM_INTERNAL_H

#include "prt_mem_external.h"
#include "prt_attr_external.h"
#include "prt_lib_external.h"
#include "prt_cpu_external.h"

/*
 * 模块内宏定义
 */
#define OS_MEM_ADDR_ALIGN_TYPE_TO_SIZE(size) (0x1UL << (U32)(size))

/* 申请一个内存块 */
typedef void *(*MemAllocFunc)(enum MoudleId mid, U8 ptNo, U32 size);

/* 申请size字节并返回指向已分配内存的指针，内存地址将按照alignPow动态对齐 */
typedef void *(*MemAllocAlignFunc)(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow);

/* 释放一个内存块  */
typedef U32 (*MemFreeFunc)(void *addr);

struct TagMemFuncLib {
    void *addr;        /* 分区起始地址 */
    MemAllocFunc alloc; /* 申请一个内存块 */
    MemAllocAlignFunc allocAlign; /* 申请size字节并返回指向已分配内存的指针，内存地址将按照alignPow动态对齐 */
    MemFreeFunc free;   /* 释放一个内存块 */
};

extern struct TagMemFuncLib g_memArithAPI; /* 算法对应API */

#endif /* PRT_MEM_INTERNAL_H */
