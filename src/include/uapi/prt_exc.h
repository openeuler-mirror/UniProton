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
 * Description: 异常模块的对外头文件。
 */
#ifndef PRT_EXC_H
#define PRT_EXC_H

#include "prt_buildef.h"
#include "prt_module.h"
#include "prt_errno.h"
#if (OS_HARDWARE_PLATFORM == OS_CORTEX_M4)
#include "./hw/armv7-m/prt_exc.h"
#endif

#if (OS_HARDWARE_PLATFORM == OS_ARMV8)
#include "./hw/armv8/os_exc_armv8.h"
#endif

#if (OS_HARDWARE_PLATFORM == OS_X86_64)
#include "./hw/x86_64/os_exc_x86_64.h"
#endif

#if (OS_HARDWARE_PLATFORM == OS_RISCV64)
#include "./hw/riscv64/os_exc_riscv64.h"
#endif

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 异常错误码: 异常模块注册异常钩子函数为空。
 *
 * 值: 0x02000a01
 *
 * 解决方案：查看注册异常钩子函数是否为空。
 */
#define OS_ERRNO_EXC_REG_HOOK_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_EXC, 0x01)

/*
 * @brief 异常处理函数类型定义。
 *
 * @par 描述
 * 通过异常处理函数类型定义异常处理函数钩子，记录异常信息。
 * @attention
 *
 * @param excInfo [IN]  类型#struct ExcInfo *，异常时寄存器信息。
 *
 * @retval OS_EXC_PROC_TYPE_RETURN，系统在发生异常后(并做完相关处理后)继续运行。
 * @retval OS_EXC_PROC_TYPE_NO_RETURN，系统在发生异常后(并做完相关处理后)进入死循环，等待重启。
 * @retval OS_EXC_PROC_TYPE_RETURN_SKIP_INST，系统在发生异常后(并做完相关处理后)跳过异常指令继续运行。
 *
 * @par 依赖
 * <li>prt_exc.h：该接口声明所在的头文件。</li>
 * @see 无
 */
typedef U32 (*ExcProcFunc)(struct ExcInfo *excInfo);

/*
 * 模块配置信息结构体。
 */
struct ExcModInfo {
    /* 异常时用户注册的函数钩子 */
    ExcProcFunc excepHook;
};

/*
 * @brief 用户注册异常处理钩子。
 *
 * @par 描述
 * 注册异常处理钩子。
 * @attention
 * <ul>
 * <li>当多次注册该钩子时，最后一次注册的钩子生效。
 * <li>注册的钩子函数不能为空，即一旦注册钩子函数，不能通过注册空函数将其取消。
 * </ul>
 *
 * @param hook [IN]  类型#ExcProcFunc，钩子函数。
 *
 * @retval #OS_OK  0x00000000，注册成功。
 * @retval #其它值，注册失败。
 *
 * @par 依赖
 * <li>prt_exc.h：该接口声明所在的头文件。</li>
 * @see 无
 */
extern U32 PRT_ExcRegHook(ExcProcFunc hook);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_EXC_H */
