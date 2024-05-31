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
 * Description: Tick中断的对外头文件。
 */
#ifndef PRT_TICK_H
#define PRT_TICK_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * Tick中断错误码：tick的周期错误
 *
 * 值: 0x02000501
 *
 * 解决方案:确认经过转换后的tick周期(多少cycle/tick)是否区间(0,0x00FFFFFF]内。
 */
#define OS_ERRNO_TICK_PERIOD OS_ERRNO_BUILD_ERROR(OS_MID_TICK, 0x01)

/*
 * Tick中断错误码：每秒的Tick中断计数配置不合法。
 *
 * 值: 0x02000502
 *
 * 解决方案:确保配置的每秒Tick中断计数小于系统时钟数，并且不等于0。
 */
#define OS_ERRNO_TICK_PER_SECOND_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_TICK, 0x02)

/*
 * Tick中断错误码：在调整Tick计数值时，输入的补偿值总大小为负值且绝对值大于正常的Tick计数值。
 *
 * 值: 0x02000503
 *
 * 解决方案:确保调整Tick计数值时，输入的补偿值总大小为负值时且它的绝对值不能大于正常的Tick计数值
 */
#define OS_ERRNO_TICK_ADJUST_VALUE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TICK, 0x03)

/*
 * tick补偿错误码：tick补偿模式不对。
 *
 * 值: 0x02000504
 *
 * 解决方案:检查tick补偿模式是否是浅睡或深睡模式
 */
#define OS_ERRNO_TICK_ADJUST_MODE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_TICK, 0x04)

/*
 * Tick中断错误码：用户注册的Tick中断钩子为空。
 *
 * 值: 0x02000505
 *
 * 解决方案: 请保证用户注册的Tick中断钩子不能为空。
 */
#define OS_ERRNO_TICK_REG_HOOK_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_TICK, 0x05)

/*
 * Tick中断模块配置信息的结构体定义。
 *
 * 保存Tick中断模块的配置项信息。
 */
struct TickModInfo {
    /* Tick中断的优先级配置 */
    U32 tickPriority;
    /* 每秒产生的TICK中断数 */
    U32 tickPerSecond;
};

/*
 * Tick补偿模式。
 */
enum SleepMode {
    OS_TYPE_LIGHT_SLEEP = 0, /* 浅睡唤醒补偿方式 */
    OS_TYPE_DEEP_SLEEP, /* 深睡唤醒补偿方式 */
    OS_TYPE_INVALID_SLEEP, /* 无效补偿方式 */
};

/*
 * @brief Tick中断用户钩子函数。
 *
 * @par 描述
 * Tick中断处理函数。
 *
 * @attention
 * <ul>
 * <li>用户调用的Tick中断处理钩子。</li>
 * <li>Tick模块裁剪开关未打开，调用此接口无效。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_tick.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
typedef void (*TickHandleFunc)(void);

/*
 * @brief 获取每秒钟对应的Tick数。
 *
 * @par 描述
 * 获取当前的tick计数。
 *
 * @attention 无
 *
 * @param 无。
 *
 * @retval  [0,0xFFFFFFFFFFFFFFFF] 当前的tick计数。
 * @par 依赖
 * <ul><li>prt_tick.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern U64 PRT_TickGetCount(void);

/*
 * @brief Tick中断处理函数。
 *
 * @par 描述
 * Tick中断处理函数。
 *
 * @attention
 * <ul>
 * <li>只有在Tick中断源由用户提供，并在Tick中断处理中调用有效。若在非tick中断处理函数中调用，可能引发tick时钟不准确</li>
 * <li>Tick模块裁剪开关未打开，调用此接口无效。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_tick.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern void PRT_TickISR(void);

/*
 * @brief 调整tick计数值
 *
 * @par 描述
 * 调整tick计数值
 *
 * @attention
 * <ul>
 * <li>该功能接口仅作统计使用（只影响PRT_TickGetCount返回值，不做调度行为控制</li>
 * <li>即不影响task delay，软件定时器，信号量/消息/时间演示等待</li>
 * </ul>
 *
 * @param tick 类型S32，tick计数补偿值，取值范围[-2^31, 2^31-1]。
 *
 * @retval #OS_ERRNO_TICK_ADJUST_VALUE_INVALID tick计数总补偿值为负数，并且绝对值大于实际的tick计数值
 * @retval #OS_OK
 * @par 依赖
 * <ul><li>prt_tick.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern U32 PRT_TickCountAdjust(S32 tick);

#if defined(OS_OPTION_TICKLESS)
/*
 * @brief 获取可休眠的tick数
 *
 * @par 描述
 * 获取可休眠的tick数。
 *
 * @attention
 * <ul>
 * <li> 调用者确保不会换核 </li>
 * </ul>
 *
 * @param minTicks 类型U32 *, 输出获取到的可休眠的tick数。
 * @param coreId   类型U32 *, 输出最近需要处理tick事务的核号。
 *
 * @retval #OS_FAIL  无事务需要响应tick，表示无可限休眠
 * @retval #OS_OK
 * @par 依赖
 * <ul><li>prt_tick.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern U32 PRT_TickLessCountGet(U32 *minTicks, U32 *coreId);

/*
 * @brief 更新tick计数值
 *
 * @par 描述
 * 更新tick计数值
 *
 * @attention
 * <ul>
 * <li> </li>
 * </ul>
 *
 * @param 
 * @param
 *
 * @retval #OS_FAIL
 * @retval #OS_OK
 * @par 依赖
 * <ul><li>prt_tick.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern U32 PRT_TickCountUpdate(enum SleepMode sleepMode, S32 ticks);

#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_TICK_H */
