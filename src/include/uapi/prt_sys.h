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
 * Description: 系统模块的对外头文件。
 */
#ifndef PRT_SYS_H
#define PRT_SYS_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 系统基本功能错误码：指针参数为空。
 *
 * 值: 0x02000001
 *
 * 解决方案: 请检查入参是否为空
 */
#define OS_ERRNO_SYS_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_SYS, 0x01)

/*
 * 系统基本功能错误码：系统主频配置非法。
 *
 * 值: 0x02000002
 *
 * 解决方案: 在prt_config.h中配置合理的系统主频。
 */
#define OS_ERRNO_SYS_CLOCK_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_SYS, 0x02)

/*
 * 系统基本功能错误码：CPUP告警被裁减
 *
 * 值: 0x02000003
 *
 * 解决方案:排除config项是否正确，该平台不支持CPUP告警
 *
 */
#define OS_ERRNO_SYS_NO_CPUP_WARN OS_ERRNO_BUILD_ERROR(OS_MID_SYS, 0x03)

/*
 * 系统基本功能错误码：设置RND值的时候入参非法。
 *
 * 值: 0x02000004
 *
 * 解决方案: 请确保设置RND值时入参合法。
 */
#define OS_ERRNO_SYS_RND_PARA_INVALID  OS_ERRNO_BUILD_ERROR(OS_MID_SYS, 0x04)

/*
 * 系统基本功能错误码：注册获取系统时间函数重复注册了
 *
 * 值: 0x02000005
 *
 * 解决方案:获取系统时间函数不允许重复注册
 *
 */
#define OS_ERRNO_SYS_TIME_HOOK_REGISTER_REPEATED OS_ERRNO_BUILD_ERROR(OS_MID_SYS, 0x05)

/*
 * 硬中断错误码：配置的中断个数不正确
 *
 * 值: 0x02000806
 *
 * 解决方案: 检查配置的中断个数是否为0或者超过了硬件范围
 */
#define OS_ERRNO_SYS_HWI_MAX_NUM_CONFIG_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x06)

/*
 * 系统初始化阶段状态
 */
/*
 * 表示初始态。
 *
 */
#define OS_DEFAULT_PHASE 0x00

/*
 * 表示进入PRT_HardBootInit。
 *
 */
#define OS_BOOT_PHASE 0x03

/*
 * 表示进入BSS段初始化。
 *
 */
#define OS_BSSINIT_PHASE 0x06

/*
 * 表示进入C lib库初始化。
 *
 */
#define OS_LIBCINIT_PHASE 0x08

/*
 * 表示系统在进行OS模块注册阶段，匹配MOUDLE_ID之后，标记进入MODULE_ID的注册。
 *
 */
#define OS_REGISTER_PHASE 0x09

/*
 * 表示系统在进行OS模块初始化阶段，匹配MOUDLE_ID之后，标记进入MODULE_ID的初始化。
 *
 */
#define OS_INITIALIZE_PHASE 0x0a

/*
 * 表示系统在进行产品驱动初始化阶段，匹配MOUDLE_ID之后，标记进入MODULE_ID的初始化。
 *
 */
#define OS_DEVDRVINIT_PHASE 0x0b

/*
 * 表示系统在进行OS启动阶段，匹配MOUDLE_ID之后，标记进入MODULE_ID的启动。
 *
 */
#define OS_START_PHASE 0x0c

/*
 * 每秒毫秒数
 */
#define OS_SYS_MS_PER_SECOND 1000

/*
 * 每秒微秒数
 */
#define OS_SYS_US_PER_SECOND 1000000

/*
 * OS版本号
 */
#define OS_SYS_OS_VER_LEN 48

/*
 * 产品版本号
 */
#define OS_SYS_APP_VER_LEN 64

/*
 * 系统模块线程类型获取枚举结构定义
 *
 * 系统模块线程类型获取
 */
enum SysThreadType {
    SYS_HWI,      /* 当前线程类型属于硬中断 */
    SYS_TASK,     /* 当前线程类型属于任务 */
    SYS_KERNEL,   /* 当前线程类型属于内核 */
    SYS_BUTT
};

/*
 * 系统模块配置信息的结构体定义。
 *
 * 保存系统模块的配置项信息。
 */
struct SysModInfo {
    /* CPU主频，时钟周期 */
    U32 systemClock;
    /* CPU type */
    U32 cpuType;
#if defined(OS_OPTION_HWI_MAX_NUM_CONFIG)
    U32 hwiMaxNum;
#endif
};

enum SysRndNumType {
    OS_SYS_RND_STACK_PROTECT,     /* 栈保护的RND值 */
    OS_SYS_RND_BUTT               /* 非法类型 */
};

/*
 * @brief 获取OS的版本号。
 *
 * @par 描述
 * 获取OS的版本号。版本编号格式为UniProton xx.xx VERSIONID(XXX)。
 *
 * @attention 无
 *
 * @param 无。
 *
 * @retval 指向OS版本号的字符串指针。
 * @par 依赖
 * <ul><li>prt_sys.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern char *PRT_SysGetOsVersion(void);

/*
 * @brief 业务给OS传递RND值，用作后续相关功能模块的保护。
 *
 * @par 描述: 业务传递RND值给OS，OS用于运行时必要的保护。
 *
 * @attention
 * <ul>
 * <li>栈保护随机值设置必须在PRT_HardBootInit中调用。</li>
 * </ul>
 *
 * @param type   [IN]  类型#enum SysRndNumType，设置的目标RND值的类型。
 * @param rndNum [IN]  类型#U32，传递的RND值。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * <ul><li>prt_sys.h：该接口声明所在的头文件。</li></ul>
 */
extern U32 PRT_SysSetRndNum(enum SysRndNumType type, U32 rndNum);

/*
 * @brief 复位单板。
 *
 * @par 描述
 * 复位单板，程序重新加载执行。
 *
 * @attention
 * <ul>
 * <li>并非直接复位单板，而是关中断，等待看门狗复位。</li>
 * <li>用户可以通过配置项OS_SYS_REBOOT_HOOK挂接复位函数。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval 无
 * @par 依赖
 * <ul><li>prt_sys.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
extern void PRT_SysReboot(void);

/*
 * @brief 获取当前核ID。
 *
 * @par 描述
 * 获取当前核ID。
 *
 * @attention
 * <ul>
 * <li>获取的核ID为硬件寄存器中的ID号。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval 物理核ID。
 * @par 依赖
 * <ul><li>prt_sys.h：该接口声明所在的头文件。</li></ul>
 * @see 无
 */
OS_SEC_ALW_INLINE INLINE U32 PRT_SysGetCoreId(void)
{
    return 0;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_SYS_H */
