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
 * Create: 2024-01-23
 * Description: 原子运算操作模块外部头文件。
 */

#ifndef PRT_ATOMIC_H
#define PRT_ATOMIC_H

#include "prt_task.h"
// #include "./common/os_atomic.h"
#include "prt_errno.h"
#include "prt_module.h"

#if(OS_HARDWARE_PLATFORM == OS_ARMV8)
#include "hw/armv8/os_atomic_armv8.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define OS_SPINLOCK_UNLOCK 0
#define OS_SPINLOCK_LOCK 1

/*
 * @ingroup PRT_atomic
 * 锁不被任何owner持有，处于空闲状态。
 */
#define OS_INVALID_LOCK_OWNER_ID 0xFFFFFFFF

/*
 * @ingroup PRT_atomic
 * 锁被非任务线程持有（中断、系统钩子等）。
 */
#define OS_LOCK_OWNER_SYS 0xFFFFFFFF

/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：初始化地址不合法
 * 
 * 值：0x02004900
 * 
 * 解决方案：请调整SPLINIT中的首地址
 */
#define OS_ERRNO_SPL_ADDR_SIZE_ILLEGAL           OS_ERRNO_BUILD_ERROR(OS_MID_SPL, 0x00)

/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：初始化地址及大小未对齐。
 * 
 * 值：0x02004902
 * 
 * 解决方案：修改初始化地址及大小使其对齐
 */
#define OS_ERRNO_SPL_ADDR_SIZE_NOT_ALIGN        OS_ERRNO_BUILD_ERROR(OS_MID_SPL, 0x02)

/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：地址大小配置不合法
 * 
 * 值：0x02004903
 * 
 * 解决方案：修改地址大小
 */
#define OS_ERRNO_SPL_SIZE_INVALID               OS_ERRNO_BUILD_ERROR(OS_MID_SPL, 0x03)

/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：输入的地址为空
 * 
 * 值：0x03004904
 * 
 * 解决方案：检查输入
 */
#define OS_ERRNO_SPL_ALLOC_ADDR_INVALID          OS_ERRNO_BUILD_ERROR(OS_MID_SPL, 0x04)

/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：锁申请地址时已无可用地址
 * 
 * 值：0x03004905
 * 
 * 解决方案：检查锁释放是否正常，或者调整初始化地址大小
 */
#define OS_ERRNO_SPL_NO_FREE                     OS_ERRNO_BUILD_ERROR(OS_MID_SPL, 0x05)

/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：锁释放地址为空
 * 
 * 值：0x03004906
 * 
 * 解决方案：检查入参是否正确，修改释放地址使之不为空
 */
#define OS_ERRNO_SPL_FREE_ADDR_INVALID            OS_ERRNO_BUILD_ERROR(OS_MID_SPL, 0x06)

// 207.5中没有
/*
 * @ingroup PRT_atomic
 * SPL地址管理错误码：OS内部初始化锁时出错
 * 
 * 值：0x03004907
 * 
 * 解决方案：检查内部内参或者自旋锁资源是否已消耗完
 */
#define OS_ERRNO_SPL_INIT_FAIL                    OS_ERRNO_BUILD_FATAL(OS_MID_SPL, 0x07)
struct OsSpinLock
{
    volatile uintptr_t rawLock;
};
/*
    @ingroup PRT_atomic

    带dbg信息的spinlock锁结构
*/
struct PrtSpinLock
{
    volatile uintptr_t rawLock;
};

/*
 * @brief 无符号32位原子读。
 *
 * @par 描述
 * 无符号32位原子读
 *
 * @attention
 * 在部分的CPU或DSP上用特殊指令实现，性能较高。
 * @li 基于性能考虑，OS对未入参地址进行校验，其地合法性由上层业务保证。
 *
 * @param  addr [I/O] 类型#U32 *，要读取的地址/
 *
 * @retval 读取的数据
 * @par 依赖
 * <ul><li>prt_atomic.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_AtomicReadU32 | PRT_AtomicReadU64
 */
OS_SEC_ALW_INLINE INLINE S32 PRT_AtomicRead32(S32 *addr)
{
    return (*addr);
}

/*
 * @brief 获取自旋锁。
 *
 * @par 描述
 * 获取自旋锁。
 *
 * @attention
 * @li 该接口用于已经关闭中断的场景，否则需要特别留意获取锁而没有关中断。
 * @li 该接口不支持自旋锁的未测功能，版本稳定后为提升性能，推荐使用该接口替换PRT_SqlIrqLock
 * @li 基于性能考虑，OS未对入参地址进行校验，其地址的合法性由上层业务保证。
 *
 * @param  spinLock [IN] 类型#struct PrtSpinLock *，自旋锁地址。
 *
 * @retval 无。
 * 
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SplUnLock
 */
extern void PRT_SplLock(struct PrtSpinLock *spinLock);

/*
 * @brief 释放自旋锁。
 *
 * @par 描述
 * 释放自旋锁。
 *
 * @attention 该接口需要核#PRT_SplLock接口成对使用，先释放锁再开中断
 * @li 基于性能考虑，OS未对入参地址进行校验，其地址的合法性由上层业务保证。
 *
 * @param  spinLock [IN] 类型#struct PrtSpinLock *，自旋锁地址。
 *
 * @retval 无。
 * 
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SplLock
 */
extern void PRT_SplUnlock(struct PrtSpinLock *spinLock);

/*
 * @brief 获取自旋锁并关闭中断使能位
 *
 * @par 描述
 * 获取自旋锁并关闭中断使能位
 *
 * @attention 
 * @li 该接口需要核#PRT_SplIrqUnlock接口成对使用。
 * @li 该接口支持spinlock维测，版本实验室阶段推荐使用该接口进行维测，但性能相对PRT_SplLock较低。
 * @li 基于性能考虑，OS未对入参地址进行校验，其地址的合法性由上层业务保证。
 *
 * @param  spinLock [IN] 类型#struct PrtSpinLock *，自旋锁地址。
 *
 * @retval #uintptr_t类型 中断状态寄存器
 *
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SplIrqUnlock
 */
extern uintptr_t PRT_SplIrqLock(struct PrtSpinLock *spinLock);

/*
 * @brief 释放自旋锁并恢复中断使能位。
 *
 * @par 描述
 * 释放自旋锁并恢复中断使能位。
 *
 * @attention 
 * @li 该接口需要核#PRT_SplIrqLock接口成对使用。
 * @li 基于性能考虑，OS未对入参地址进行校验，其地址的合法性由上层业务保证。
 *
 * @param  spinLock [IN] 类型#struct PrtSpinLock *，自旋锁地址。
 * @param  intSave [IN] 类型#uintptr_t，中断句柄，自旋锁时的返回值。
 * 
 * @retval 无
 *
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SplIrqLock
 */
extern void PRT_SplIrqUnlock(struct PrtSpinLock *spinLock, uintptr_t intSave);

/*
 * @brief 自旋锁地址申请。
 *
 * @par 描述
 * 自旋锁地址申请。
 *
 * @attention 
 * @li 该接口需要和#PRT_SpinlockFree接口成对使用。
 *
 * @param  addr [IN] 类型#volatile uintptr_t *，存放锁地址的地址。
 * 
 * @retval #OS_ERROR_SPL_ALLOC_ADDR_INVALID         0X02004704, 输入地址为空。
 * @retval #OS_ERROR_SPL_NO_FREE         0X02004705, 没有空闲的锁地址分配。
 * @retval #OS_OK         0X00000000, 成功。
 *
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SpinlockFree
 */
extern U32 PRT_SpinlockAlloc(volatile uintptr_t *addr);

/*
 * @brief 自旋锁地址释放。
 *
 * @par 描述
 * 自旋锁地址释放。
 *
 * @attention 
 * @li 该接口需要和#PRT_SpinlockAlloc接口成对使用。
 *
 * @param  addr [IN] 类型#uintptr_t *，PRT_SpinlockAlloc申请出来的地址。
 * 
 * @retval #OS_OK         0X00000000, 成功。
 *
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SpinlockAlloc
 */
extern U32 PRT_SpinlockFree(uintptr_t addr);

/*
 * @brief 自旋锁的锁值初始化。
 *
 * @par 描述
 * 自旋锁的锁值初始化。
 *
 * @attention 
 * @li OS的自旋锁功能的锁值含义：0表示未锁，1表示上锁，建议调用该接口进行初始化，
 *     因为锁地址包含还维测字段，若锁空间不够会则进入fatal error异常。
 * @li 对于入参spinLock，OS已做基本校验，无法校验所有非法地址，其合法性由业务保证
 *
 * @param  addr [IN] 类型#struct PrtSpinLock *，自旋锁地址。
 * 
 * @retval 无
 *
 * @par 依赖
 * @li prt_atomic.h：该接口声明所在的头文件。
 * @see PRT_SpinlockDelete
 */
extern U32 PRT_SplLockInit(struct PrtSpinLock *spinLock);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif  /* __cplusplus */

#endif  /*PRT_ATOMIC_H*/