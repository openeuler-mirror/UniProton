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
 * Description: 内存基本功能对外头文件。
 */
#ifndef PRT_MEM_H
#define PRT_MEM_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */
/*
 * 内存错误码：动态内存释放时，发现内存越界。
 *
 * 值: 0x02000101
 *
 * 解决方案：使用时注意内存申请时大小，使用大小不要超过内存本身大小。
 */
#define OS_ERRNO_MEM_OVERWRITE OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x01)

/*
 * 内存错误码：释放的地址为空。
 *
 * 值: 0x02000102
 *
 * 解决方案：请检查释放的内存块地址是否正确。
 */
#define OS_ERRNO_MEM_FREE_ADDR_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x02)

/*
 * 内存错误码：内存初始化时，地址为空，初始化失败。
 *
 * 值: 0x02000103
 *
 * 解决方案: 内存初始化时，地址不能为空。
 */
#define OS_ERRNO_MEM_INITADDR_ISINVALID OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x03)

/*
 * 内存错误码：内存申请时申请的大小太大(可能为负值)。
 *
 * 值: 0x02000104
 *
 * 解决方案: 增大分区大小，或减小要申请的内存大小。
 */
#define OS_ERRNO_MEM_ALLOC_SIZETOOLARGE OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x04)

/*
 * 内存错误码：内存初始化时，内存分区大小非4字节对齐。
 *
 * 值: 0x02000105
 *
 * 解决方案: 内存分区大小需4字节对齐。
 */
#define OS_ERRNO_MEM_INITSIZE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x05)

/*
 * 内存错误码：内存初始化时，地址要4字节对齐，初始化失败。
 *
 * 值: 0x02000106
 *
 * 解决方案: 内存初始化时，地址要4字节对齐。
 */
#define OS_ERRNO_MEM_INITADDR_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x06)

/*
 * 内存错误码：分区初始化时的分区大小太小。
 *
 * 值: 0x02000107
 *
 * 解决方案: 初始化分区大小改大。
 */
#define OS_ERRNO_MEM_PTCREATE_SIZE_ISTOOSMALL OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x07)

/*
 * 内存错误码：分区初始化时的分区大小太大。
 *
 * 值: 0x02000108
 *
 * 解决方案: 初始化分区大小改小。
 */
#define OS_ERRNO_MEM_PTCREATE_SIZE_ISTOOBIG OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x08)

/*
 * 内存错误码：申请的内存块大小为0。
 *
 * 值: 0x02000109
 *
 * 解决方案：请检查申请内存大小的有效性。
 */
#define OS_ERRNO_MEM_ALLOC_SIZE_ZERO OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x09)

/*
 * 内存错误码：对齐方式不合法。
 *
 * 值: 0x0200010b
 *
 * 解决方案：请检查入参对齐方式。
 */
#define OS_ERRNO_MEM_ALLOC_ALIGNPOW_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x0a)

/*
 * 动态内存错误码：动态内存释放时要释放的内存块的头被破坏，或已释放。
 *
 * 值: 0x0200010d
 *
 * 解决方案: 保证对内存写操作时，不要出现写越界。
 *
 */
#define OS_ERRNO_MEM_FREE_SH_DAMAGED OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x0b)

/*
 * OS缺省私有FSC内存错误码：从私有FSC分区中申请内存时，找不到空闲内存，申请失败。
 *
 * 值: 0x0200010f
 *
 * 解决方案: 增大私有FSC内存分区大小。
 */
#define OS_ERRNO_FSCMEM_ALLOC_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_MEM, 0x0c)

/*
 * 系统缺省的内存分区数量。
 */
#define OS_MEM_DEFAULT_PT0 0
#define OS_MEM_DEFAULT_PT1 1
#define OS_MEM_DEFAULT_PTNUM 2

/*
 * 缺省私有FSC内存分区。
 */
#define OS_MEM_DEFAULT_FSC_PT (OS_MEM_DEFAULT_PT0)

/*
 * 申请的内存地址对齐标准:4/8字节对齐。
 */
#define OS_MEM_ADDR_ALIGN sizeof(uintptr_t)

/*
 * 内存算法类型。
 */
enum MemArith {
    MEM_ARITH_FSC,          // 私有FSC算法
    MEM_ARITH_BUTT          // 内存算法非法
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

/*
 * @brief 向已创建的指定分区申请内存。
 *
 * @par 描述
 * <li>在分区号为ptNo的分区中，申请大小为size的内存。</li>
 * @attention
 * <ul>
 * <li>申请内存时的分区号，根据实际创建的分区号来使用。</li>
 * <li>调用函数后，注意判断返回的地址是否为空以避免后续访问空指针。</li>
 * <li>对于FSC内存算法，申请到的内存首地址是按4字节对齐的 </li>
 * <li>如果内存申请失败，返回值为NULL，而导致失败的原因将记录在错误处理空间中。</li>
 * </ul>
 *
 * @param mid  [IN]  类型#U32，申请的模块号。
 * @param ptNo [IN]  类型#U8，分区号，范围[0,#OS_MEM_MAX_PT_NUM+2)。
 * @param size [IN]  类型#U32，申请的大小。
 *
 * @retval #NULL  0，申请失败。
 * @retval #!NULL 内存首地址值。
 * @par 依赖
 * <ul><li>prt_mem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_MemFree
 */
extern void *PRT_MemAlloc(U32 mid, U8 ptNo, U32 size);

/*
 * @brief 向已创建的指定分区申请指定大小且指定对齐方式的内存块。
 *
 * @par 描述
 * <li>在分区号为ptNo的分区中，申请大小为size的内存，对齐方式为alignPow。</li>
 * @attention
 * <ul>
 * <li>申请内存分区时的分区号，根据实际创建的分区号来使用。</li>
 * <li>调用函数后，注意判断返回的地址是否为空以避免后续访问空指针。</li>
 * <li>如果内存申请失败，返回值为NULL，而导致失败的原因将记录在错误处理空间中。</li>
 * </ul>
 *
 * @param mid      [IN]  类型#U32，申请的模块号。
 * @param ptNo     [IN]  类型#U8，分区号，范围[0,#OS_MEM_MAX_PT_NUM+2)。
 * @param size     [IN]  类型#U32，申请的大小。
 * @param alignPow [IN]  类型#enum MemAlign，动态对齐。
 *
 * @retval #NULL  0，申请失败。
 * @retval #!NULL 内存首地址值。
 * @par 依赖
 * <ul><li>prt_mem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_MemFree
 */
extern void *PRT_MemAllocAlign(U32 mid, U8 ptNo, U32 size, enum MemAlign alignPow);

/*
 * @brief 释放申请的内存。
 *
 * @par 描述
 * 该接口根据内存块的地址addr，找到该内存块所属的内存分区，再根据分区号和用户使用的地址addr释放该内存。
 * @attention
 * <ul>
 * <li>如果返回值不是#OS_OK，则内存不会被释放。</li>
 * <li>被破坏的内存不能被释放。</li>
 * <li>对于入参addr，OS已做基本校验，无法校验所有非法地址，其合法性由业务保证。</li>
 * </ul>
 *
 * @param mid  [IN]  类型#U32，要释放的模块号。
 * @param addr [IN]  类型#void *，释放的地址。
 *
 * @retval #OS_OK  0x00000000，内存释放成功。
 * @retval #其它值，释放失败。
 * @par 依赖
 * <ul><li>prt_mem.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_MemAlloc
 */
extern U32 PRT_MemFree(U32 mid, void *addr);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_MEM_H */
