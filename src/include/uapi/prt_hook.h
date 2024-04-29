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
 * Description: 内核钩子头文件。
 */
#ifndef PRT_HOOK_H
#define PRT_HOOK_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * HOOK错误码：HOOK类型错误。
 *
 * 值: 0x02000901
 *
 * 解决方案：确认输入钩子是否为有效的OS_HOOK_TYPE_E类型。
 */
#define OS_ERRNO_HOOK_TYPE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x01)

/*
 * HOOK错误码：内存不足。
 *
 * 值: 0x02000902
 *
 * 解决方案：增加缺省分区大小。
 */
#define OS_ERRNO_HOOK_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x02)

/*
 * HOOK错误码：HOOK指针空。
 *
 * 值: 0x02000903
 *
 * 解决方案：检查入参的钩子，NULL指针不允许进行添加或删除。
 */
#define OS_ERRNO_HOOK_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x03)

/*
 * HOOK错误码：HOOK已存在。
 *
 * 值: 0x02000904
 *
 * 解决方案：确认该钩子之前是否已经成功注册，所以再次注册失败。
 */
#define OS_ERRNO_HOOK_EXISTED OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x04)

/*
 * HOOK错误码：HOOK已满。
 *
 * 值: 0x02000905
 *
 * 解决方案：若为多钩子可以增大配置，或删掉一部分钩子；若为单钩子则表示重复注册，建议先删除再注册。
 */
#define OS_ERRNO_HOOK_FULL OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x05)

/*
 * HOOK错误码：HOOK不存在。
 *
 * 值: 0x02000906
 *
 * 解决方案：确认该钩子之前是否已经成功注册。
 */
#define OS_ERRNO_HOOK_NOT_EXISTED OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x06)

/*
 * HOOK错误码：HOOK配置个数为0。
 *
 * 值: 0x02000907
 *
 * 解决方案：该类型钩子个数配置为0，添加或者删除某类型钩子前，需要先对其个数进行正确配置。
 */
#define OS_ERRNO_HOOK_NOT_CFG OS_ERRNO_BUILD_ERROR(OS_MID_HOOK, 0x07)

/*
 * 用户可使用钩子类型枚举。
 */
enum HookType {
    OS_HOOK_HWI_ENTRY = 0,  // 硬中断进入钩子
    OS_HOOK_HWI_EXIT,       // 硬中断退出钩子
    OS_HOOK_TSK_SWITCH,     // 任务切换钩子
    OS_HOOK_IDLE_PERIOD,    // IDLE钩子
    OS_HOOK_LAST_WORDS,     // 临终遗言钩子
    OS_HOOK_TSK_CREATE,     //任务创建钩子
    OS_HOOK_TSK_DELETE,     //任务切换钩子
    OS_HOOK_TYPE_NUM,       // 钩子总数
};

/*
 * 钩子模块配置信息的结构体定义。
 */
struct HookModInfo {
    U8 maxNum[(U32)OS_HOOK_TYPE_NUM];
};

/*
 * 钩子添加内部接口
 */
extern U32 OsHookAdd(enum HookType hookType, OsVoidFunc hook);

/*
 * 钩子删除内部接口
 */
extern U32 OsHookDel(enum HookType hookType, OsVoidFunc hook);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_HOOK_H */
