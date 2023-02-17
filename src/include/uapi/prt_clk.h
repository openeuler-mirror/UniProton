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
 * Description: 时钟模块的对外头文件。
 */
#ifndef PRT_CLK_H
#define PRT_CLK_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * @brief 获取当前的64位time stamp计数(即系统运行的cycles)。
 *
 * @par 描述
 * 获取当前的64位time stamp计数(即系统运行的cycles)。
 *
 * @attention
 * <ul>
 * <li>获取的是64bit cycles 数据。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval [0,0xFFFFFFFFFFFFFFFF] 系统当前的cycle数
 * @par 依赖
 * <ul><li>prt_clk.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_CycleCountGet32()
 */
extern U64 PRT_ClkGetCycleCount64(void);

/*
 * @brief 转换cycle为毫秒。
 *
 * @par 描述
 * 转换cycle为毫秒。
 *
 * @attention 无
 *
 * @param cycle [IN]  类型#U64，cycle数。
 *
 * @retval [0,0xFFFFFFFFFFFFFFFF] 转换后的毫秒数。
 * @par 依赖
 * <ul><li>prt_clk.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_ClkCycle2Us
 */
extern U64 PRT_ClkCycle2Ms(U64 cycle);

/*
 * @brief 转换cycle为微秒。
 *
 * @par 描述
 * 转换cycle为微秒。
 *
 * @attention 无
 *
 * @param cycle [IN]  类型#U64，cycle数。
 *
 * @retval [0,0xFFFFFFFFFFFFFFFF] 转换后的微秒数。
 * @par 依赖
 * <ul><li>prt_clk.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_ClkCycle2Ms
 */
extern U64 PRT_ClkCycle2Us(U64 cycle);

/*
 * @brief 延迟时间(单位微秒)。
 *
 * @par 描述
 * 延迟时间(单位微秒)。
 *
 * @attention 无
 *
 * @param delay [IN]  类型#U32，延迟微秒数。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_clk.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_ClkDelayMs
 */
extern void PRT_ClkDelayUs(U32 delay);

/*
 * @brief 延迟时间(单位毫秒)。
 *
 * @par 描述
 * 延迟时间(单位毫秒)。
 *
 * @param delay [IN]  类型#U32，延迟毫秒数。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_clk.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_ClkDelayUs
 */
extern void PRT_ClkDelayMs(U32 delay);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_CLK_H */
