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
 * Description: 背景任务模块的对外头文件。
 */
#ifndef PRT_IDLE_H
#define PRT_IDLE_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * @brief IDLE循环钩子的类型定义。
 *
 * @par 描述
 * 用户通过IDLE循环钩子的函数类型定义函数，系统在进入IDLE循环之前调用该钩子。
 *
 * @attention 无。
 *
 * @param 无。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_idle.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef void (*IdleHook)(void);

/*
 * @brief 注册IDLE循环进入前调用的钩子。
 *
 * @par 描述
 * 为本核注册IDLE循环进入前调用的钩子，该钩子只会被调用一次。
 *
 * @attention
 * <ul>
 * <li>注册的钩子只在进入IDLE循环前执行一次。</li>
 * <li>若任务未裁剪，则作用的是任务IDLE循环。</li>
 * <li>IDLE任务钩子中使用矢量寄存器需要在前置钩子中调用#PRT_TaskCpSaveCfg接口设置矢量操作上下文保护区。</li>
 * </ul>
 *
 * @param hook [IN]  类型#IdleHook，钩子函数。
 *
 * @retval #OS_OK                          0x00000000，操作成功。
 *
 * @par 依赖
 * <ul><li>prt_idle.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_IdleAddHook
 */
extern U32 PRT_IdleAddPrefixHook(IdleHook hook);

/*
 * @brief 注册IDLE循环中调用的钩子
 *
 * @par 描述
 * 注册在IDLE任务中调用的钩子函数。
 *
 * @attention
 * <ul>
 * <li>钩子中不能调用引起任务阻塞或切换的函数。</li>
 * </ul>
 *
 * @param hook [OUT] 类型#IdleHook，IDLE钩子函数，该参数不能为空。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * <ul><li>prt_idle.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_IdleDelHook
 */
extern U32 PRT_IdleAddHook(IdleHook hook);

/*
 * @brief 删除IDLE循环中调用的钩子
 *
 * @par 描述
 * 删除在IDLE任务中调用的钩子函数。
 *
 * @attention
 * <ul>
 * 无
 * </ul>
 *
 * @param hook [OUT] 类型#IdleHook，IDLE钩子函数，该参数不能为空。
 *
 * @par 依赖
 * <ul><li>prt_idle.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_IdleAddHook
 */
extern U32 PRT_IdleDelHook(IdleHook hook);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_IDLE_H */