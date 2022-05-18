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
 * Description: LIB模块内部头文件
 */
#ifndef PRT_LIB_EXTERNAL_H
#define PRT_LIB_EXTERNAL_H

#include "securec.h"
#include "prt_sys.h"

/* 对齐操作API */
#define OS_ADDR_OVERTURN_CHK(addr, size) (((size) != 0) && (((addr) + (size) - 1) < (addr)))
#define OS_ALIGN_CHK(addr, align)       (((uintptr_t)(addr) & ((align) - 1)) == 0)
#define OS_NOT_ALIGN_CHK(addr, align)   (!OS_ALIGN_CHK(addr, align))
#define OS_ALIGN(addr, align)           ((uintptr_t)(addr) & ((align) - 1))

#define OS_32BIT_MOD(num)               ((num) % 32)

#define OS_GET_64BIT_HIGH_32BIT(num)    ((U32)((num) >> 32))
#define OS_GET_64BIT_LOW_32BIT(num)     ((U32)((num) & 0xFFFFFFFFU))

#define OS_GET_32BIT_HIGH_8BIT(num)     (((num) >> 24) & 0xffU)
#define OS_GET_32BIT_LOW_8BIT(num)      ((num) & 0xffU)

#define OS_GET_32BIT_HIGH_16BIT(num)    ((num) >> 16)
#define OS_GET_32BIT_LOW_16BIT(num)     ((num) & 0xFFFFU)

#define OS_GET_16BIT_HIGH_8BIT(num)     ((num) >> 8)

#define OS_GET_8BIT_HIGH_4BIT(num)      ((num) >> 4)
#define OS_GET_8BIT_LOW_4BIT(num)       ((num) & 0xFU)

#define OS_GET_32BIT_ARRAY_INDEX(num)   ((num) >> 5)
#define OS_GET_32BIT_ARRAY_BASE(num)    ((num) << 5)

#define OS_32BIT_MASK(bit)              (1U << OS_32BIT_MOD(bit))
#define OS_32BIT_VERSE_MASK(bit)        (~(1U << OS_32BIT_MOD(bit)))

#define OS_GET_BIT_IN_WORD(num)         ((num) & 0x1FUL)

#define OS_64BIT_SET(high, low)         (((U64)(high) << 32) + (U64)(low))
#define OS_LMB32                        31

#define OS_MAX_U32 0xFFFFFFFFU
#define OS_MAX_U16 0xFFFFU
#define OS_MAX_U12 0xFFFU
#define OS_MAX_U8 0xFFU
#define OS_MAX_U4 0xFU

#define OS_MAX_S8 0xFF

#define OS_BYTES_PER_DWORD              (sizeof(U64))
#define OS_BYTES_PER_WORD               (sizeof(U32))
#define OS_BITS_PER_BYTE                8

#define OS_BITNUM_2_MASK(bitNum) ((1U << (bitNum)) - 1)
#define OS_WORD_ALLBIT_MASK 0xFFFFFFFFU

#define OS_IS_VISIBLE_CHAR(word) ((((word) >= 'a') && ((word) <= 'z')) || (((word) >= 'A') && ((word) <= 'Z')) || \
                                  ((word) == '_') || (((word) <= '9') && ((word) >= '0')))

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define OS_DECIMAL    10
#define OS_OCTAL      8
#define OS_HEX        16

#define OS_BIT0_MASK  (0x1U << 0)
#define OS_BIT1_MASK  (0x1U << 1)
#define OS_BIT2_MASK  (0x1U << 2)
#define OS_BIT3_MASK  (0x1U << 3)
#define OS_BIT4_MASK  (0x1U << 4)
#define OS_BIT5_MASK  (0x1U << 5)
#define OS_BIT6_MASK  (0x1U << 6)
#define OS_BIT7_MASK  (0x1U << 7)
#define OS_BIT8_MASK  (0x1U << 8)
#define OS_BIT9_MASK  (0x1U << 9)
#define OS_BIT10_MASK (0x1U << 10)
#define OS_BIT11_MASK (0x1U << 11)
#define OS_BIT12_MASK (0x1U << 12)
#define OS_BIT13_MASK (0x1U << 13)
#define OS_BIT14_MASK (0x1U << 14)
#define OS_BIT15_MASK (0x1U << 15)
#define OS_BIT16_MASK (0x1U << 16)
#define OS_BIT17_MASK (0x1U << 17)
#define OS_BIT18_MASK (0x1U << 18)
#define OS_BIT19_MASK (0x1U << 19)
#define OS_BIT20_MASK (0x1U << 20)
#define OS_BIT21_MASK (0x1U << 21)
#define OS_BIT22_MASK (0x1U << 22)
#define OS_BIT23_MASK (0x1U << 23)
#define OS_BIT24_MASK (0x1U << 24)
#define OS_BIT25_MASK (0x1U << 25)
#define OS_BIT26_MASK (0x1U << 26)
#define OS_BIT27_MASK (0x1U << 27)
#define OS_BIT28_MASK (0x1U << 28)
#define OS_BIT29_MASK (0x1U << 29)
#define OS_BIT30_MASK (0x1U << 30)
#define OS_BIT31_MASK (0x1U << 31)

#define OS_BIT_SET_VALUE(bit) (0x1UL << (bit))

#define BNUM_TO_WNUM(bnum)   (((bnum) + 31) >> 5)
#define OS_WORD_BIT_NUM      32
#define OS_WORD_BIT_POW      5
#define OS_DWORD_BIT_NUM     64
#define OS_DWORD_BIT_POW     6
#define OS_HALF_WORD_BIT_NUM 16
#define OS_GET_WORD_NUM_BY_PRIONUM(prio) (((prio) + 0x1f) >> 5)  // 通过支持的优先级个数计算需要多少个word表示

#ifndef OS_STATIC_NO_INLINE
#define OS_STATIC_NO_INLINE static __noinline __attribute__((noinline))
#endif

#ifndef ATTR_ALIGN_128
#define ATTR_ALIGN_128 __attribute__((aligned(128))) /* 表示128 字节对齐 */
#endif

#ifndef ATTR_ALIGN_64
#define ATTR_ALIGN_64 __attribute__((aligned(64)))   /* 表示64 字节对齐 */
#endif

#ifndef ATTR_ALIGN_32
#define ATTR_ALIGN_32 __attribute__((aligned(32)))   /* 表示32 字节对齐 */
#endif

#ifndef ATTR_ALIGN_16
#define ATTR_ALIGN_16 __attribute__((aligned(16)))   /* 表示16 字节对齐 */
#endif

#ifndef ATTR_ALIGN_8
#define ATTR_ALIGN_8 __attribute__((aligned(8)))     /* 表示8字节对齐 */
#endif

#ifndef ATTR_ALIGN_4
#define ATTR_ALIGN_4 __attribute__((aligned(4)))     /* 表示4字节对齐 */
#endif

#define OS_SYS_UINT_IDX(coreId) ((U32)(coreId) >> 5)  // 一个U32表示32个核
#define OS_SYS_UINT_LCOREID_OFFSET(coreId) (((coreId) & 0x1fUL))  // 核号在一个U32里的偏移

/*
 * 模块间宏定义
 */
extern void OsAdd64(U32 *low, U32 *high, U32 oldLow, U32 oldHigh);
extern void OsSub64(U32 *low, U32 *high, U32 oldLow, U32 oldHigh);
extern U32 OsGetLmb1(U32 value);

#endif /* PRT_LIB_EXTERNAL_H */
