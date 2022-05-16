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
 * Description: 错误处理头文件。
 */
#ifndef PRT_ERR_H
#define PRT_ERR_H

#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * @brief 错误处理钩子函数。
 *
 * @par 描述
 * 错误处理的钩子函数，用户自定义的错误处理函数必须要符合该钩子函数的参数要求。
 *
 * @attention 无。
 *
 * @param fileName [IN]  类型#const char *，出错文件名。
 * @param lineNo   [IN]  类型#U32，出错的行号。
 * @param errorNo  [IN]  类型#U32，用户定义的错误码。
 * @param paraLen  [IN]  类型#U32，入参para的长度。
 * @param para     [IN]  类型#void *，记录用户对于错误的描述或其他。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_err.h：该接口声明所在的头文件。</li>
 * @see 无
 */
typedef void (*ErrHandleFunc)(const char *fileName, U32 lineNo, U32 errorNo, U32 paraLen, void *para);

/*
 * @brief 处理错误。
 *
 * @par 描述
 * 用户或者OS内部使用该函数回调#PRT_ErrRegHook中注册的钩子函数。另外，OS内部使用此接口时，还会记录相关错误码。
 * @attention
 * <ul>
 * <li>系统不会做入参检测，用户PRT_ErrHandle会全部当做钩子入参输入。</li>
 * <li>用户调用PRT_ErrHandle接口时，只会回调用户注册钩子函数，不会进行错误码记录
 * （致命错误也不会记录Trace轨迹和触发异常）。</li>
 * </ul>
 *
 * @param fileName [IN]  类型#const char *，错误发生的文件名，可以用__FILE__作为输入。
 * @param lineNo   [IN]  类型#U32，错误发生所在的行号，可以用__LINE__作为输入。
 * @param errorNo  [IN]  类型#U32，用户输入的错误码。
 * @param paraLen  [IN]  类型#U32，入参para的长度。
 * @param para     [OUT] 类型#void *，记录用户对于错误的描述或其他(地址)。
 *
 * @retval #OS_OK  0x00000000，处理错误成功。
 * @par 依赖
 * <li>prt_err.h: 该接口声明所在的头文件。</li>
 * @see 无
 */
extern U32 PRT_ErrHandle(const char *fileName, U32 lineNo, U32 errorNo, U32 paraLen, void *para);

/*
 * @brief 注册用户错误处理的钩子函数。
 *
 * @par 描述
 * 注册hook作为用户钩子函数，在调用PRT_ErrHandle接口进行错误处理时对该钩子进行调用。
 * @attention
 * <ul>
 * <li>若入参hook为NULL,则为取消钩子。</li>
 * <li>不允许重复或覆盖注册。</li>
 * <li>用户定义的钩子函数必须符合#PRT_ERRORHANDLE_FUNC定义的形式，而且只能定义一个钩子函数。</li>
 * <li>用户调用PRT_ErrRegHook注册回调钩子函数时，钩子函数里面不能有调用PRT_ErrHandle
 * 或者调用OS函数发生上报错误码的情况，否则可能会进入死循环，需由用户保证。</li>
 * <li>用户调用PRT_ErrRegHook注册回调钩子函数时，钩子函数里面如有单次上报的错误信息（只有第一次调用会执行），
 * 那么会先记录钩子中错误信息，再记录发生错误时的错误信息。</li>
 * </ul>
 *
 * @param hook [IN]  类型#ErrHandleFunc，用户钩子函数的宏定义。
 *
 * @retval #OS_OK  0x00000000，注册成功。
 * @retval #其它值，注册失败。
 * @par 依赖
 * <li>prt_err.h：该接口声明所在的头文件。</li>
 * @see 无
 */
extern U32 PRT_ErrRegHook(ErrHandleFunc hook);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_ERR_H */
