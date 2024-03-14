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
 * Description: 硬中断模块的对外头文件。
 */
#ifndef PRT_HWI_H
#define PRT_HWI_H

#include "prt_module.h"
#include "prt_errno.h"
#include "prt_buildef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * IPI触发类型。
 */
enum OsHwiIpiType {
    OS_TYPE_TRIGGER_BY_MASK = 0, /* 通过mask确定需要触发的目标核 */
    OS_TYPE_TRIGGER_TO_OTHER, /* 触发除本核外的其他核 */
    OS_TYPE_TRIGGER_TO_SELF, /* 触发本核 */
    OS_TYPE_TRIGGER_BUTT /* 非法 */
};

/*
 * 支持的SGI中断编号为[0,15]
 * 可用的核间中断号定义。
 */
#define OS_HWI_IPI_NO_00                                  0

/*
 * 可用的核间中断号定义[OS占用1号:触发它核响应一次调度的IPI中断号]。
 */
#define OS_HWI_IPI_NO_01                                  1

/*
 * 可用的核间中断号定义[OS占用2号:一个核异常后将其它核停住的IPI中断号]。
 */
#define OS_HWI_IPI_NO_02                                  2

/*
 * 可用的核间中断号定义[OS占用3号:响应tick中断的核触发它核的模拟tickIPI中断号]。
 */
#define OS_HWI_IPI_NO_03                                  3

/*
 * 可用的核间中断号定义[OS占用4号:核间通信用于核间通知的中断号]。
 */
#define OS_HWI_IPI_NO_04                                  4

/*
 * 可用的核间中断号定义。
 */
#define OS_HWI_IPI_NO_05                                  5
#define OS_HWI_IPI_NO_06                                  6

/*
 * 可用的核间中断号定义[OS占用7号:openamp要求Linux和从核使用同一个IPI，Linux核目前只能使用7号]。
 */
#define OS_HWI_IPI_NO_07                                  7

/*
 * 可用的核间中断号定义。
 */
#define OS_HWI_IPI_NO_08                                  8
#define OS_HWI_IPI_NO_09                                  9
#define OS_HWI_IPI_NO_10                                  10
#define OS_HWI_IPI_NO_011                                 11
#define OS_HWI_IPI_NO_012                                 12
#define OS_HWI_IPI_NO_013                                 13
#define OS_HWI_IPI_NO_014                                 14
#define OS_HWI_IPI_NO_015                                 15

/*
 * 硬中断错误码：中断号非法。
 *
 * 值: 0x02000801
 *
 * 解决方案：确保中断号合法，中断号请查看硬件手册。
 */
#define OS_ERRNO_HWI_NUM_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x01)

/*
 * 硬中断错误码：优先级非法。
 *
 * 值: 0x02000802
 *
 * 解决方案：确保优先级合法。
 */
#define OS_ERRNO_HWI_PRI_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x02)

/*
 * 硬中断错误码：硬中断已被创建或相应中断向量号已被其它中断占用。
 *
 * 值: 0x02000803
 *
 * 解决方案：更换中断号
 */
#define OS_ERRNO_HWI_ALREADY_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x03)

/*
 * 硬中断错误码：硬中断处理函数为空。
 *
 * 值: 0x02000804
 *
 * 解决方案：传入非空的有效处理函数
 */
#define OS_ERRNO_HWI_PROC_FUNC_NULL OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x04)

/*
 * 硬中断错误码：未创建的硬中断被响应。
 *
 * 值: 0x03000805
 *
 * 解决方案：先创建硬中断，然后使能并触发该中断使其得到响应
 */
#define OS_ERRNO_HWI_UNCREATED OS_ERRNO_BUILD_FATAL(OS_MID_HWI, 0x05)

/*
 * 硬中断错误码：设置硬中断属性时，属性设置与之前设置值不一致
 *
 * 值: 0x02000806
 *
 * 解决方案：确认当前设置属性值与之前是否一致。如果确需修改属性，请先删除该硬中断。
 */
#define OS_ERRNO_HWI_ATTR_CONFLICTED OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x06)

/*
 * 硬中断错误码：组合型中断创建失败，为组合型中断节点申请系统默认私有FSC内存失败，或申请中断描述信息失败。
 *
 * 值: 0x02000807
 *
 * 解决方案: 增大系统默认私有FSC分区大小
 */
#define OS_ERRNO_HWI_MEMORY_ALLOC_FAILED OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x07)

/*
 * 硬中断错误码：组合型中断函数注册失败，该组合型中断已创建了相同的中断处理函数。
 *
 * 值: 0x02000808
 *
 * 解决方案: 更换中断处理函数
 */
#define OS_ERRNO_HWI_COMBINEHOOK_ALREADY_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x08)

/*
 * 硬中断错误码：创建的中断函数即不是独立型，也不是组合型
 *
 * 值: 0x02000809
 *
 * 解决方案: 硬中断模式只能设置为独立型或组合型
 */
#define OS_ERRNO_HWI_MODE_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x09)

/*
 * 硬中断错误码：删除未创建或者已经被删除的硬中断。
 *
 * 值: 0x0200080a
 *
 * 解决方案: 删除已创建并且未被删除的硬中断
 */
#define OS_ERRNO_HWI_DELETED OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0a)

/*
 * 硬中断错误码：未进行硬中断模式设置。
 *
 * 值: 0x0200080b
 *
 * 解决方案: 调用中断创建函数前，需要先调用中断模式设置函数，进行模式参数设置
 */
#define OS_ERRNO_HWI_MODE_UNSET OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0b)

/*
 * 硬中断错误码：硬中断触发接口入参错误，输入无效的核号。
 *
 * 值: 0x0200080c
 *
 * 解决方案: 输入本核核号
 */
#define OS_ERRNO_HWI_CORE_ID_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0c)

/*
 * 硬中断错误码：硬件上报错误中断。
 *
 * 值: 0x0200080d
 *
 * 解决方案：无。
 */
#define OS_ERRNO_HWI_HW_REPORT_HWINO_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0d)

/*
 * 系统基本功能错误码：注册不可删除的中断失败
 *
 * 值: 0x0300080e
 *
 * 解决方案: 请确保传是独立型中断，或者修改OS_HWI_INTERNAL_NUM值
 */
#define OS_ERROR_HWI_INT_REGISTER_FAILED OS_ERRNO_BUILD_FATAL(OS_MID_HWI, 0x0e)

/*
 * 硬中断错误码：中断内存资源申请失败
 *
 * 值: 0x0200080f
 *
 * 解决方案: 检查默认分区大小配置是否正确
 */
#define OS_ERRNO_HWI_RESOURCE_ALLOC_FAILED OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0f)

/*
 * 核间中断错误码：中断目标核不支持1-N
 *
 * 值: 0x02000810
 *
 * 解决方案：目标核掩码只描述1个目标核，不能描述多个目标核
 */
#define OS_ERRNO_MULTI_TARGET_CORE OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x10)

/*
 * 硬中断错误码：删除os内部硬中断。
 *
 * 值: 0x02000811
 *
 * 解决方案：不允许删除os内部硬中断
 */
#define OS_ERRNO_HWI_DELETE_INT OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x11)

/*
 * 硬中断错误码：硬中断地址信息配置错误
 *
 * 值: 0x02000812
 *
 * 解决方案: 根据核手册正确配置OS_GIC_BASE_ADDR/OS_GICR_OFFSET/OS_GICR_STRIDE
 */
#define OS_ERROR_HWI_BASE_ADDR_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x12)

/*
 * 系统基本功能错误码：关中断超时注册形参非法
 *
 * 值: 0x02000813
 *
 * 解决方案: 请确认传入的形参非空
 */
#define OS_ERROR_HWI_INT_LOCK_REG_PARA_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x13)
/*
 * 系统基本功能错误码：ARM核间触发中断号非法
 *
 * 值: 0x02000814
 *
 * 解决方案: ARM平台确保软件触发中断传入的中断为SGI中断
 */
#define OS_ERRNO_HWI_TRIGGER_HWINUM_NOT_SGI OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x14)

/*
 * 系统基本功能错误码：ARM核间触发中断类型非法
 *
 * 值: 0x02000815
 *
 * 解决方案: ARM平台确保软件触发中断传入的触发类型正确
 */
#define OS_ERRNO_HWI_TRIGGER_TYPE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x15)

/*
 * 系统基本功能错误码：ARM核间触发中断核掩码非法
 *
 * 值: 0x02000816
 *
 * 解决方案: ARM平台确保软件触发中断传入的触发核掩码正确
 */
#define OS_ERRNO_HWI_TRIGGER_MASK_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x16)

/*
 * 系统基本功能错误码：中断绑定的核掩码非法
 *
 * 值: 0x02000817
 *
 * 解决方案: 确保中断绑定传入的核掩码正确
 */
#define OS_ERRNO_HWI_AFFINITY_MASK_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x17)

/*
 * 系统基本功能错误码：中断绑定的中断号时SGI
 *
 * 值: 0x02000818
 *
 * 解决方案: 确保中断绑定的中断号不是SGI
 */
#define OS_ERRNO_HWI_AFFINITY_HWINUM_SGI OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x18)

/*
 * 硬中断优先级的类型定义。
 */
typedef U16 HwiPrior;

/*
 * 硬中断模式配置信息的类型定义。
 */
typedef U16 HwiMode;

/*
 * 硬中断处理函数的参数类型定义。
 */
typedef uintptr_t HwiArg;

/*
 * 硬中断号的类型定义。
 */
typedef U32 HwiHandle;

/*
 * 组合型硬中断。
 */
#define OS_HWI_MODE_COMBINE 0x8000

/*
 * 独立型硬中断。
 */
#define OS_HWI_MODE_ENGROSS 0x4000

/*
 * 缺省硬中断模式。
 */
#define OS_HWI_MODE_DEFAULT OS_HWI_MODE_ENGROSS

/*
 * 普通硬中断。
 */
#define OS_HWI_TYPE_NORMAL 0x00

/*
 * 中断属性组装宏(共8bit)。
 */
#define OS_HWI_ATTR(mode, type) (HwiMode)((HwiMode)(mode) | (HwiMode)(type))

/*
 * @brief 硬中断处理函数的类型定义。
 *
 * @par 描述
 * 通过硬中断处理函数的类型定义硬中断处理函数，在硬中断触发时调用该中断处理函数。
 *
 * @attention 无。
 *
 * @param  param1 [IN] 类型#HwiArg，硬中断处理函数的参数。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef void (*HwiProcFunc)(HwiArg);

/*
 * @brief 硬中断调用处理函数钩子函数类型定义。
 *
 * @par 描述
 * 用户通过硬中断调用钩子处理函数类型定义硬中断调用处理函数钩子，在硬中断调用处理函数时，调用该钩子。
 * @attention 无。
 *
 * @param hwiNum [IN]  类型#U32，硬中断号。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 */
typedef void (*HwiEntryHook)(U32 hwiNum);

/*
 * @brief 硬中断退出处理函数钩子函数类型定义。
 *
 * @par 描述
 * 用户通过硬中断退出钩子处理函数类型定义硬中断退出处理函数钩子，在硬中断退出处理函数时，调用该钩子。
 * @attention 无。
 *
 * @param hwiNum [IN]  类型#U32，硬中断号。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 */
typedef void (*HwiExitHook)(U32 hwiNum);

/*
 * @brief 设置硬中断属性接口。
 *
 * @par 描述
 * 在创建硬中断前，必须要配置好硬中断的优先级和模式，包括独立型（#OS_HWI_MODE_ENGROSS）和
 * 组合型（#OS_HWI_MODE_COMBINE）两种配置模式。
 *
 * @attention
 * <ul>
 * <li>OS已经占用的不能被使用</li>
 * </ul>
 *
 * @param hwiNum  [IN]  类型#HwiHandle，硬中断号。
 * @param hwiPrio [IN]  类型#HwiPrior，硬中断优先级。
 * @param mode    [IN]  类型#HwiMode，设置的中断模式，为独立型(#OS_HWI_MODE_ENGROSS)或者组合型(#OS_HWI_MODE_COMBINE)。
 *
 * @retval #OS_OK  0x00000000，硬中断属性设置成功。
 * @retval #其它值，属性设置失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiCreate
 */
extern U32 PRT_HwiSetAttr(HwiHandle hwiNum, HwiPrior hwiPrio, HwiMode mode);

/*
 * @brief 创建硬中断函数。
 *
 * @par 描述
 * 注册硬中断的处理函数。
 *
 * @attention
 * <ul>
 * <li>在调用该函数之前，请先确保已经设置了中断属性。</li>
 * <li>硬中断创建成功后，并不使能相应向量的中断，需要显式调用#PRT_HwiEnable单独使能。</li>
 * </ul>
 *
 * @param hwiNum  [IN]  类型#HwiHandle，硬中断号。
 * @param handler [IN]  类型#HwiProcFunc，硬中断触发时的处理函数。
 * @param arg     [IN]  类型#HwiArg，调用硬中断处理函数时传递的参数。
 *
 * @retval #OS_OK  0x00000000，硬中断创建成功。
 * @retval #其它值，创建失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiDelete
 */
extern U32 PRT_HwiCreate(HwiHandle hwiNum, HwiProcFunc handler, HwiArg arg);

/*
 * @brief 删除硬中断函数。
 *
 * @par 描述
 * 屏蔽相应硬中断或事件，取消硬中断处理函数的注册。
 *
 * @attention
 * <ul>
 * <li>不能删除OS占用的中断号。</li>
 * </ul>
 *
 * @param hwiNum [IN]  类型#HwiHandle，硬中断号。
 *
 * @retval #OS_OK  0x00000000，硬中断删除成功。
 * @retval #其它值，删除失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiCreate
 */
extern U32 PRT_HwiDelete(HwiHandle hwiNum);

/*
 * @brief 激活指定核号内的硬中断。
 *
 * @par 描述
 * 激活指定核号内的软件可触发的硬中断
 *
 * @attention
 *
 * @param dstCore [IN]  类型#U32，目标核号。目前只支持指定为本核。
 * @param hwiNum  [IN]  类型#HwiHandle，硬中断号，只支持软件可触发的中断号。
 *
 * @retval #OS_OK  0x00000000，硬中断激活成功。
 * @retval #其它值，激活失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 */
extern U32 PRT_HwiTrigger(U32 dstCore, HwiHandle hwiNum);

/*
 * @brief 清空中断请求位。
 *
 * @par 描述
 * 清除所有的中断请求位。即放弃当前已触发中断的的响应。
 *
 * @attention
 * <ul>
 * 清除所有的中断请求位(对于NMI中断无效)。
 * </ul>
 *
 * @param 无。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiClearPendingBit
 */
extern void PRT_HwiClearAllPending(void);

/*
 * @brief 清除硬中断的Pending位。
 *
 * @par 描述
 * 显式清除硬中断或事件的请求位，因为有的硬件响应中断后不会自动清Pending位。
 *
 * @attention
 *
 * @param hwiNum [IN]  类型#HwiHandle，硬中断号。
 *
 * @retval #OS_OK  0x00000000，硬中断请求位清除成功。
 * @retval #其它值，清除失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiCreate
 */
extern U32 PRT_HwiClearPendingBit(HwiHandle hwiNum);

/*
 * @brief 屏蔽指定的硬中断。
 *
 * @par 描述
 * 禁止核响应指定硬中断的请求。
 *
 * @attention
 *
 * @param hwiNum [IN]  类型#HwiHandle，依据不同的芯片，硬中断号或中断向量号，见注意事项。
 *
 * @retval #OS_OK  0x00000000，硬中断去使能成功。
 * @retval #其它值，硬中断去使能失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiEnable
 */
extern U32 PRT_HwiDisable(HwiHandle hwiNum);

/*
 * @brief 使能指定的硬中断。
 *
 * @par 描述
 * 允许核响应指定硬中断的请求。
 *
 * @attention
 * <ul>
 * <li>对于不同芯片，此返回值代表的意义有所差异，差异细节见下面返回值说明</li>
 * </ul>
 *
 * @param hwiNum [IN]  类型#HwiHandle，依据不同的芯片，硬中断号或中断向量号，见注意事项。
 *
 * @retval #OS_OK  0x00000000，硬中断使能成功。
 * @retval #其它值，硬中断使能失败。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiDisable
 */
extern U32 PRT_HwiEnable(HwiHandle hwiNum);

/*
 * @brief 添加硬中断进入钩子
 *
 * @par 描述
 * 添加硬中断进入钩子。该钩子函数在进入硬中断ISR前被调用。
 *
 * @attention
 * <ul>
 * <li>不同钩子函数间执行的先后顺序，不应当存在依赖关系。</li>
 * <li>不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。</li>
 * <li>最大支持钩子数需静态配置</li>
 * </ul>
 *
 * @param hook [IN]  类型#HwiEntryHook，中断进入钩子函数。
 *
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiDelEntryHook | PRT_HookAdd | PRT_HwiAddExitHook
 */
extern U32 PRT_HwiAddEntryHook(HwiEntryHook hook);

/*
 * @brief 删除硬中断进入钩子
 *
 * @par 描述
 * 删除硬中断进入钩子。该钩子函数将停止在进入硬中断ISR前的调用。
 *
 * @attention
 * <ul>
 * <li>不同钩子函数间执行的先后顺序，不应当存在依赖关系。</li>
 * <li>不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。</li>
 * <li>最大支持钩子数需静态配置</li>
 * </ul>
 *
 * @param hook [IN]  类型#HwiEntryHook，中断进入钩子函数。
 *
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiAddEntryHook | PRT_HookDel
 */
extern U32 PRT_HwiDelEntryHook(HwiEntryHook hook);

/*
 * @brief 添加硬中断退出钩子
 *
 * @par 描述
 * 添加硬中断退出钩子。该钩子函数在退出硬中断ISR后被调用。
 *
 * @attention
 * <ul>
 * <li>不同钩子函数间执行的先后顺序，不应当存在依赖关系。</li>
 * <li>不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。</li>
 * <li>最大支持钩子数需静态配置</li>
 * </ul>
 *
 * @param hook [IN]  类型#HwiExitHook，中断退出钩子函数。
 *
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiDelExitHook | PRT_HookAdd | PRT_HwiAddEntryHook
 */
extern U32 PRT_HwiAddExitHook(HwiExitHook hook);

/*
 * @brief 删除硬中断退出钩子
 *
 * @par 描述
 * 删除硬中断退出钩子。该钩子函数将停止在退出硬中断ISR后的调用。
 *
 * @attention
 * <ul>
 * <li>不同钩子函数间执行的先后顺序，不应当存在依赖关系。</li>
 * <li>不应在钩子函数里调用可能引起线程调度或阻塞的OS接口。</li>
 * <li>最大支持钩子数需静态配置</li>
 * </ul>
 *
 * @param hook [IN]  类型#HwiExitHook，中断退出钩子函数。
 *
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiAddExitHook | PRT_HookDel
 */
extern U32 PRT_HwiDelExitHook(HwiExitHook hook);

/*
 * @brief 开中断。
 *
 * @par 描述
 * 开启全局可屏蔽中断。
 *
 * @attention 中断服务函数里慎用该接口，会引起中断优先级反转
 *
 * @param 无。
 *
 * @retval 开启全局中断前的中断状态值。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiLock | PRT_HwiRestore
 */
extern uintptr_t PRT_HwiUnLock(void);

/*
 * @brief 关中断。
 *
 * @par 描述
 * 关闭全局可屏蔽中断。
 *
 * @attention 在关全局中断后，禁止调用引起内核调度的相关接口，如PRT_TaskDelay接口
 *
 * @param 无。
 *
 * @retval 关闭全局中断前的中断状态值。
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiUnLock | PRT_HwiRestore
 */
extern uintptr_t PRT_HwiLock(void);

/*
 * @brief 恢复中断状态接口。
 *
 * @par 描述
 * 恢复原中断状态寄存器。
 *
 * @attention
 * <ul>
 * <li>该接口必须和关闭全局中断或者是开启全局中断接口成对使用，以关全局中断或者开全局中断操作的返回值为入参</li>
 * <li>以保证中断可以恢复到关全局中断或者开全局中断操作前的状态</li>
 * </ul>
 * @param intSave [IN]  类型#uintptr_t，关全局中断PRT_HwiLock和开全局中断PRT_HwiUnLock的返回值。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_hwi.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_HwiUnLock | PRT_HwiLock
 */
extern void PRT_HwiRestore(uintptr_t intSave);

#if defined(OS_OPTION_SMP)
extern U32 PRT_HwiSetAffinity(U32 hwiNum, U32 coreMask);
extern U32 PRT_HwiMcTrigger(enum OsHwiIpiType type, U32 coreMask, U32 hwiNum);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_HWI_H */
