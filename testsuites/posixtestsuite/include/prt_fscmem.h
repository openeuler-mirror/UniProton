/*
 * Copyright (c) 2022-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-08-15
 * Description: prt_mem.h for posix memory interface testsuite
 */
#ifndef _PRT_FSCMEM_H
#define _PRT_FSCMEM_H

typedef unsigned char U8;
typedef unsigned short U16;
typedef unsigned int U32;
typedef unsigned long long U64;
typedef signed char S8;
typedef signed short S16;
typedef signed int S32;
typedef signed long long S64;

/* FSC算法块控制头结构，注意各成员顺序是和其他算法保持一致偏移的，不能随便改动，保持ptNo和其他算法偏移一致 */
struct TagFscMemCtrl {
    struct TagFscMemCtrl *next;
    // 块大小
    unsigned int size;
    // 若前面相邻的物理块空闲，则此字段记录前面空闲块大小，否则为OS_FSC_MEM_PREV_USED
    unsigned int prevSize;
    // 空闲时为上一个控制块地址
    struct TagFscMemCtrl *prev;
};

/*
 * 内存对齐方式。
 */
enum MemAlign {
    MEM_ADDR_ALIGN_004 = 2, /* 4字节对齐 */
    MEM_ADDR_ALIGN_008 = 3, /* 8字节对齐 */
    MEM_ADDR_ALIGN_016 = 4, /* 16字节对齐 */
    MEM_ADDR_ALIGN_032 = 5, /* 32字节对齐 */
    MEM_ADDR_ALIGN_064 = 6, /* 64字节对齐 */
    MEM_ADDR_ALIGN_128 = 7, /* 128字节对齐 */
    MEM_ADDR_ALIGN_256 = 8, /* 256字节对齐 */
    MEM_ADDR_ALIGN_512 = 9, /* 512字节对齐 */
    MEM_ADDR_ALIGN_1K = 10, /* 1K字节对齐 */
    MEM_ADDR_ALIGN_2K = 11, /* 2K字节对齐 */
    MEM_ADDR_ALIGN_4K = 12, /* 4K字节对齐 */
    MEM_ADDR_ALIGN_8K = 13, /* 8K字节对齐 */
    MEM_ADDR_ALIGN_16K = 14, /* 16K字节对齐 */
    MEM_ADDR_BUTT /* 字节对齐非法 */
};

#define OS_MEM_ADDR_ALIGN sizeof(unsigned int)

#define OS_FSC_MEM_LAST_IDX   		31
#define OS_FSC_MEM_SIZE_ALIGN 		OS_MEM_ADDR_ALIGN
#define OS_FSC_MEM_MAXVAL     		((1U << OS_FSC_MEM_LAST_IDX) - OS_FSC_MEM_SIZE_ALIGN)

#define OS_FSC_MEM_USED_HEAD_SIZE   (sizeof(struct TagFscMemCtrl))
#define OS_FSC_MEM_TAIL_SIZE        (sizeof(unsigned int))

#define OS_FSC_MEM_TAIL_MAGIC       0xABCDDCBA

/*
 * free ptr
 */
#define PTS_FREE(ptr) do {      \
        if ((ptr) != NULL) {    \
            free(ptr);          \
            (ptr) = NULL;       \
        }                       \
    } while(0)


size_t malloc_usable_size(void *p);

#endif
