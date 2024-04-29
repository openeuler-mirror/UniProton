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
 * Create: 2024-01-26
 * Description: armv8平台原子运算操作模块外部头文件
 */
#ifndef OS_ATOMIC_ARMV8_H
#define OS_ATOMIC_ARMV8_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

struct Atomic32 {
    volatile U32 counter;
};

struct Atomtic64 {
    volatile U64 counter;
};

OS_SEC_ALW_INLINE INLINE void OsArmPrefecthw(volatile void *ptr)
{
    OS_EMBED_ASM("prfm  pstl1strm, %0"::"Q"(ptr):"memory", "cc");
}

OS_SEC_ALW_INLINE INLINE S64 OsAtomic64Add(S64 i, struct Atomtic64 *v)
{
    uintptr_t temp;
    S64 ret;

    OsArmPrefecthw(&v->counter);

    OS_EMBED_ASM("1:ldxr            %0, %2          \n"
                 "  add             %0, %0, %3      \n"
                 "  stlxr           %w1, %0, %2     \n"
                 "  cbnz            %w1, 1b         \n"
                 "  dmb ish"
                 : "=&r" (ret), "=&r" (temp), "+Q" (v->counter)
                 : "r" (i)
                 : "memory");
    return  ret;
}

OS_SEC_ALW_INLINE INLINE S32 OsAtomic32Add(S32 i, struct Atomic32 *v)
{
    uintptr_t temp;
    S32 ret;

    OsArmPrefecthw(&v->counter);

    OS_EMBED_ASM("1:ldxr            %w0, %2          \n"
                 "  add             %w0, %w0, %w3      \n"
                 "  stlxr           %w1, %w0, %2     \n"
                 "  cbnz            %w1, 1b         \n"
                 "  dmb ish"
                 : "=&r" (ret), "=&r" (temp), "+Q" (v->counter)
                 : "r" (i)
                 : "memory");
    return ret;
}

OS_SEC_ALW_INLINE INLINE U64 OsLdrExd(volatile void *ptr)
{
    (void)ptr;
    return 0;
}

OS_SEC_ALW_INLINE INLINE S32 OsStrExd(U64 val, volatile void *ptr)
{
    (void)ptr;
    return (S32)val;
}

/*
 * @brief 有符号32位变量的原子加，并返回累加后的值
 *
 * @par 描述
 * 有符号32位变量的原子加，并返回累加后的值
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S32*, 要累加变量的地址。
 * @param[in]       incr 类型#S32, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAdd32Rtn| PRT_AtomicFetchAndAdd32
 */
OS_SEC_ALW_INLINE INLINE S32 PRT_AtomicAdd32Rtn(S32 *ptr, S32 incr)
{
    return OsAtomic32Add(incr, (struct Atomic32 *)ptr);
}

/*
 * @brief 无符号32位变量的原子加，并返回累加后的值
 *
 * @par 描述
 * 有符号32位变量的原子加，并返回累加后的值
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S32*, 要累加变量的地址。
 * @param[in]       incr 类型#S32, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddU32| PRT_AtomicFetchAndAddU32
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_AtomicAddU32Rtn(U32 *ptr, U32 incr)
{
    return (U32)OsAtomic32Add((S32)incr, (struct Atomic32 *)ptr);
}

/*
 * @brief 有符号64位变量的原子加，并返回累加后的值
 *
 * @par 描述
 * 有符号32位变量的原子加，并返回累加后的值
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S64*, 要累加变量的地址。
 * @param[in]       incr 类型#S64, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAdd64Rtn| PRT_AtomicFetchAndAdd64
 */
OS_SEC_ALW_INLINE INLINE S64 PRT_AtomicAdd64Rtn(S64 *atomic, S64 incr)
{
    return OsAtomic64Add(incr, (struct Atomtic64 *)atomic);
}

/*
 * @brief 无符号64位变量的原子加，并返回累加后的值
 *
 * @par 描述
 * 有符号64位变量的原子加，并返回累加后的值
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S64*, 要累加变量的地址。
 * @param[in]       incr 类型#S64, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddU64 | PRT_AtomicFetchAndAddU64
 */
OS_SEC_ALW_INLINE INLINE U64 PRT_AtomicAddU64Rtn(U64 *atomic, U64 incr)
{
    return (U64)OsAtomic64Add((S64)incr, (struct Atomtic64 *)atomic);
}

/*
 * @brief 有符号32位变量的原子加
 *
 * @par 描述
 * 有符号32位变量的原子加
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S32*, 要累加变量的地址。
 * @param[in]       incr 类型#S32, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddS32Rtn| PRT_AtomicFetchAndAddS32
 */
OS_SEC_ALW_INLINE INLINE void PRT_AtomicAdd32(S32 *ptr, S32 incr)
{
    (void)OsAtomic32Add(incr, (struct Atomic32 *)ptr);
}

/*
 * @brief 无符号32位变量的原子加
 *
 * @par 描述
 * 有符号32位变量的原子加
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S32*, 要累加变量的地址。
 * @param[in]       incr 类型#S32, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddU32Rtn| PRT_AtomicFetchAndAddU32
 */
OS_SEC_ALW_INLINE INLINE void PRT_AtomicAddU32(U32 *ptr, U32 incr)
{
    (void)OsAtomic32Add((S32)incr, (struct Atomic32 *)ptr);
}

/*
 * @brief 有符号64位变量的原子加
 *
 * @par 描述
 * 有符号32位变量的原子加
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S64*, 要累加变量的地址。
 * @param[in]       incr 类型#S64, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddS64| PRT_AtomicFetchAndAddS64
 */
OS_SEC_ALW_INLINE INLINE void PRT_AtomicAddS64(S64 *atomic, S64 incr)
{
    (void)OsAtomic64Add(incr, (struct Atomtic64 *)atomic);
}

/*
 * @brief 无符号64位变量的原子加
 *
 * @par 描述
 * 有符号64位变量的原子加
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S64*, 要累加变量的地址。
 * @param[in]       incr 类型#S64, 要累加的数值。
 *
 * @retval 变量累加后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddU64Rtn | PRT_AtomicFetchAndAddU64
 */
OS_SEC_ALW_INLINE INLINE void PRT_AtomicAddU64(U64 *atomic, U64 incr)
{
    (void)OsAtomic64Add((S64)incr, (struct Atomtic64 *)atomic);
}

/*
 * @brief 32位原子交换，并返回交换前的值
 *
 * @par 描述
 * 32位原子交换，并返回交换前的值
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U32*, 要交换变量的地址。
 * @param[in]       newValue 类型#U32，要交换的值。
 *
 * @retval 变量交换前的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicSwap64
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_AtomicSwap32(U32 *ptr, U32 newValue)
{
    U32 tmp = 0;
    U32 ret = 1;

    (void)(ptr);
    OS_EMBED_ASM("  prfm        pstl1strm, [%2]     \n"
                 "1: ldaxr      %w0, [%2]           \n"
                 "  stxr       %w1, %w3, [%2]      \n"
                 "  cbnz        %w1, 1b             \n"
                 : "=&r" (tmp), "=&r" (ret), "+r" (ptr)
                 : "r" (newValue));
    return tmp;
}

/*
 * @brief 64位原子交换，并返回交换前的值
 *
 * @par 描述
 * 64位原子交换，并返回交换前的值
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U64*, 要交换变量的地址。
 * @param[in]       newValue 类型#U64，要交换的值。
 *
 * @retval 变量交换前的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicSwap64
 */
OS_SEC_ALW_INLINE INLINE U64 PRT_AtomicSwap64(U64 *ptr, U64 newValue)
{
    U64 tmp = *ptr;
    *ptr = newValue;
    return tmp;
}

/*
 * @brief 完成32位无符号的变量与指定内存的值比较，并在相等的情况下赋值
 *
 * @par 描述
 * 完成32位无符号的变量与指定内存的值比较，并在相等的情况下赋值，相等时赋值并返回1，不相等时返回零。
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U32*, 要比较/赋值变量的地址。
 * @param[in]       oldVal 类型#U32，要比较的值。
 * @param[in]       newVal 类型#U32，相等时要写入的新值。
 *
 * @retval 1，相等并赋值
 * @retval 0，不相等。
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicCompareAndStore64
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_AtomicCompareAndStore32(U32 *ptr, U32 oldVal, U32 newVal)
{
    U32 tmp = 0;
    U32 ret = 1;

    (void)(ptr);
    OS_EMBED_ASM("  prfm        pstl1strm, [%2]     \n"
                 "1: ldaxr      %w0, [%2]           \n"
                 "   cmp        %w0, %w3            \n"
                 "   bne        2f                  \n"
                 "  stxr        %w1, %w4, [%2]      \n"
                 "  cbnz        %w1, 1b             \n"
                 "2:                                \n"
                 : "=&r" (tmp), "+&r" (ret), "+&r" (ptr)
                 : "r" (oldVal), "r" (newVal));
    return !ret;
}

/*
 * @brief 有符号32位变量的原子加，并返回累加前的值
 *
 * @par 描述
 * 有符号32位变量的原子加，并返回累加前的值
 * 
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#S32*, 要累加变量的地址。
 * @param[in]       incr 类型#S32, 要累加的数值。
 *
 * @retval 变量累加前的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddS32| PRT_AtomicAddS32Rtn
 */
OS_SEC_ALW_INLINE INLINE S32 PRT_AtomicFetchAndAddS32(S32 *ptr, S32 incr){
    OsArmPrefecthw(ptr);
    return OsAtomic32Add(incr, (struct Atomic32 *)ptr);
}

/*
 * @brief 无符号的32位原子或操作
 *
 * @par 描述
 * 无符号的32位原子或操作
 * 
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  addr 类型#U32*, 要执行逻辑或变量的地址。
 * @param[in]       incr 类型#U32, 要逻辑或的值。
 *
 * @retval 执行或操作以后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_AtomicOr(U32 *addr, U32 val)
{
    U32 ret = *addr;

    *addr = ret | val;

    return ret;
}

/*
 * @brief 无符号的32位原子与操作
 *
 * @par 描述
 * 无符号的32位原子与操作
 * 
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  addr 类型#U32*, 要执行逻辑与变量的地址。
 * @param[in]       incr 类型#U32, 要逻辑与的值。
 *
 * @retval 执行或操作以后的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_AtomicAnd(U32 *addr, U32 val)
{
    U32 ret = *addr;

    *addr = ret & val;

    return ret;
}

/*
 * @brief 完成64位无符号的变量与指定内存的值比较，并在相等的情况下赋值
 *
 * @par 描述
 * 完成64位无符号的变量与指定内存的值比较，并在相等的情况下赋值，相等时赋值并返回1，不相等时返回零。
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U64*, 要比较/赋值变量的地址。
 * @param[in]       oldVal 类型#U64，要比较的值。
 * @param[in]       newVal 类型#U64，相等时要写入的新值。
 *
 * @retval 1，相等并赋值
 * @retval 0，不相等。
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicCompareAndStore64
 */
OS_SEC_ALW_INLINE INLINE U64 PRT_AtomicCompareAndStore64(U64 *ptr, U64 oldVal, U64 newVal)
{
    if(oldVal == newVal){
        *ptr = oldVal;
        return 1;
    }
    return 0;
}
/*
 * @brief 无符号32位变量的原子加，并返回累加前的值
 *
 * @par 描述
 * 无符号32位变量的原子加，并返回累加前的值
 * 
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U32*, 要累加变量的地址。
 * @param[in]       incr 类型#U32, 要累加的数值。
 *
 * @retval 变量累加前的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddU32| PRT_AtomicAddU32Rtn
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_AtomicFetchAndAddU32(U32 *ptr, U32 incr)
{
    OsArmPrefecthw(ptr);
    return (U32)OsAtomic32Add(incr, (struct Atomic32 *)ptr);
}

/*
 * @brief 无符号64位变量的原子加，并返回累加前的值
 *
 * @par 描述
 * 无符号64位变量的原子加，并返回累加前的值
 * 
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U64*, 要累加变量的地址。
 * @param[in]       incr 类型#U64, 要累加的数值。
 *
 * @retval 变量累加前的值
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicAddU64| PRT_AtomicAddU64Rtn
 */
OS_SEC_ALW_INLINE INLINE U64 PRT_AtomicFetchAndAddU64(U64 *ptr, U64 incr)
{
    OsArmPrefecthw(ptr);
    return (U64)OsAtomic64Add((S64)incr, (struct Atomtic64 *)ptr);
}

/*
 * @brief 无符号64位原子读
 *
 * @par 描述
 * 无符号64位原子读
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 *
 * @param[in, out]  ptr 类型#U64*, 要读取的地址。
 *
 * @retval 读取的数据
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicRead32 | PRT_AtomicReadU32 | PRT_AtomicRead64
 */
OS_SEC_ALW_INLINE INLINE U64 PRT_AtomicReadU64(U64 *ptr)
{
    U64 tmp = 0;
    do {
        tmp = (U64)OsLdrExd(ptr);
    } while (OsStrExd(tmp, ptr));

    OS_EMBED_ASM("DMB OSHLD" : : : "memory");
    return tmp;
}
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* OS_ATOMIC_ARMV8_H */
