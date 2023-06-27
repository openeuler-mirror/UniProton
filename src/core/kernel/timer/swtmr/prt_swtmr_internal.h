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
 * Description: swtmr模块的模块内头文件
 */
#ifndef PRT_SWTMR_INTERNAL_H
#define PRT_SWTMR_INTERNAL_H

#include "prt_lib_external.h"
#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_swtmr_external.h"
#include "prt_sys_external.h"
#include "prt_timer_external.h"
#include "prt_err_external.h"
#include "prt_tick_external.h"
#include "prt_mem_external.h"
#include "prt_list_external.h"
#include "prt_cpu_external.h"

/*
 * 模块内宏定义
 */
#define OS_SWTMR_SORTLINK_LEN   64 /* SortLink数组长度 */
#define OS_SWTMR_SORTLINK_MASK  0x3FUL

#define OS_TICK_SWTMR_GROUP_ID 0 /* 软件定时器组ID号 */

#define OS_SWTMR_STATUS_DEFAULT 0
#define OS_SWTMR_STATUS_MASK 0xfU
#define OS_SWTMR_PRE_STATUS_MASK 0xf0U

/*
 * @brief 获取低位的值。
 *
 * @par 描述
 * 宏定义，定义完成从一个32位的数获取低26位的值
 *
 * @attention 无
 * @param  num [IN] 源数。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define UWROLLNUM(num) ((num) & 0x03ffffffU)
/*
 * @brief 获取高位的值。
 *
 * @par 描述
 * 宏定义，定义完成从一个32位的数获取高位的值
 *
 * @attention 无
 * @param  num [IN] 源数。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define UWSORTINDEX(num) ((num) >> 26)
/*
 * @brief 定义高位赋值。
 *
 * @par 描述
 * 宏定义，屏蔽高6位，把值与到高6位
 *
 * @attention 无
 * @param  num [IN] 被赋值的变量。
 * @param  value [IN] 待赋的值或变量。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define EVALUATE_H(num, value) ((num) = (((num) & 0x03ffffffU) | ((value) << 26)))

/*
 * @brief 定义低位赋值。
 *
 * @par 描述
 * 宏定义，定义完成一个32位的数低位赋值的功能，高位不变。
 *
 * @attention 无
 * @param  num [IN] 被赋值的变量。
 * @param  value [IN] 待赋的值或变量。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define EVALUATE_L(num, value) ((num) = (((num) & 0xfc000000U) | (value)))

/*
 * @brief 定义两个数低位相减的功能。
 *
 * @par 描述
 * 宏定义，定义完成一个32位的数低位相加，结果存放到第一个参数，H(num1) = H(num1) - H(num2)。
 * 如 两个定时器控制块中sortData低位的rollNum进行相加。
 *
 * @attention 无
 * @param  num1 [IN] 被加数。
 * @param  num2 [IN] 加数。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define UWROLLNUMADD(num1, num2) ((num1) = (((num1) & 0xfc000000U) | (UWROLLNUM(num1) + UWROLLNUM(num2))))

/*
 * @brief 定义两个数低位相减的功能。
 *
 * @par 描述
 * 宏定义，定义完成一个32位的数低位相减，结果存放到第一个参数，H(num1) = H(num1) - H(num2)。
 * 如 两个定时器控制块中sortData低位的rollNum进行相减。
 *
 * @attention 无
 * @param  num1 [IN] 被减数。
 * @param  num2 [IN] 减数。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define UWROLLNUMSUB(num1, num2) ((num1) = (((num1) & 0xfc000000U) | (UWROLLNUM(num1) - UWROLLNUM(num2))))

/*
 * @brief 定义低位减1的功能。
 *
 * @par 描述
 * 宏定义，定义完成一个32位的数低位自减1。
 * 如 定时器控制块中sortData低位的rollNum进行自减1操作
 *
 * @attention 无
 * @param  num [IN] 低为待自减1的数。
 *
 * @retval 无
 * @par 依赖
 * <li>prt_swtmr_external.h: 该宏定义所在的头文件。</li></ul>
 * @see UWROLLNUMSUB | UWROLLNUMADD | EVALUATE_L | EVALUATE_H | UWROLLNUM | UWSORTINDEX。
 */
#define UWROLLNUMDEC(num) ((num) = ((num) - 1))

/*
 * 模块内全局变量声明
 */
/* 软件定时器Sortlink */
extern struct TagSwTmrSortLinkAttr g_tmrSortLink;
/* 软件定时器空闲链表 */
extern struct TagSwTmrCtrl *g_tmrFreeList;

/*
 * 模块内函数声明
 */
/*
 * Function   : OsSwTmrResInit
 * Description: 软件定时器模块的初始化接口
 * Input      : none
 * Output     : none
 * Return     : OS_OK                           -- 初始化成功
 *              OS_ERRNO_SWTMR_NO_MEMORY             -- 初始化内存不足
 */
extern U32 OsSwTmrResInit(void);

/*
 * Function   : OsSwTmrRun
 * Description: 软件定时器模块的Tick中断运行接口
 * Input      : none
 * Output     : none
 * Return     : OS_OK                 -- 成功
 */
extern void OsSwTmrScan(void);

/*
 * Function   : OsSwTmrGetRemainTick
 * Description: 获取软件定时器剩余Tick数的内部接口
 * Input      : swtmr [IN] 类型#struct TagSwTmrCtrl *，定时器指针
 * Output     : none
 * Return     : 软件定时器剩余TICK数
 */
extern U32 OsSwTmrGetRemainTick(struct TagSwTmrCtrl *swtmr);

/*
 * Function   : OsSwTmrCreateTimer
 * Description: 软件定时器创建函数
 * Input      : none
 * Output     : none
 * Return     : OS_OK                           -- 初始化成功
 *              OS_ERRNO_SWTMR_NO_MEMORY             -- 初始化内存不足
 */
extern U32 OsSwTmrCreateTimer(struct TimerCreatePara *createPara, TimerHandle *tmrHandle);

/*
 * Function   : OsSwTmrStartTimer
 * Description: 软件定时器启动函数
 * Input      : none
 * Output     : none
 * Return     : OS_OK                           -- 初始化成功
 *              OS_ERRNO_SWTMR_NO_MEMORY             -- 初始化内存不足
 */
extern U32 OsSwTmrStartTimer(TimerHandle tmrHandle);

/*
 * Function   : OsSwTmrStopTimer
 * Description: 软件定时器停止函数
 * Input      : none
 * Output     : none
 * Return     : OS_OK                           -- 初始化成功
 *              OS_ERRNO_SWTMR_NO_MEMORY             -- 初始化内存不足
 */
extern U32 OsSwTmrStopTimer(TimerHandle tmrHandle);

/*
 * Function   : OsSwTmrRestartTimer
 * Description: 软件定时器重启函数
 * Input      : none
 * Output     : none
 * Return     : OS_OK                           -- 初始化成功
 *              OS_ERRNO_SWTMR_NO_MEMORY             -- 初始化内存不足
 */
extern U32 OsSwTmrRestartTimer(TimerHandle tmrHandle);

/*
 * Function   : OsSwTmrDeleteTimer
 * Description: 软件定时器删除函数
 * Input      : none
 * Output     : none
 * Return     : OS_OK                           -- 初始化成功
 *              OS_ERRNO_SWTMR_NO_MEMORY             -- 初始化内存不足
 */
extern U32 OsSwTmrDeleteTimer(TimerHandle tmrHandle);

/*
 * Function   : OsSwTmrStart
 * Description: 软件定时器的启动接口
 * Input      : swtmr  [IN] 类型#struct TagSwTmrCtrl*，         需要启动的定时器
 *              interval[IN] 类型#U32，                定时器计时时长
 * Output     : none
 * Return     : none
 * Other      : None
 */
extern void OsSwTmrStart(struct TagSwTmrCtrl *swtmr, U32 interval);

/*
 * Function   : OsSwTmrStop
 * Description: 软件定时器的暂停内部接口
 * Input      : swtmr  [IN] 类型#struct TagSwTmrCtrl*，          需要启动的定时器
 *              reckonOff [IN] 类型#UINT16                 计算剩余时间开关
 * Output     : none
 * Return     : OS_OK                                   -- 暂停定时器成功
 * Other      : None
 */
extern void OsSwTmrStop(struct TagSwTmrCtrl *swtmr, bool reckonOff);

/*
 * 模块内内联函数定义
 */
/*
 * Function   : OsSwTmrDelete
 * Description: 软件定时器的删除内部接口
 * Input      : swtmr  [IN] 类型#struct TagSwTmrCtrl*，         需要删除的定时器
 * Output     : none
 * Return     : none
 * Other      : None
 */
extern void OsSwTmrDelete(struct TagSwTmrCtrl *swtmr);

/*
 * Function    : OsSwTmrGetOverrun
 * Description : 查询软件定时器剩余超时时间
 * Input       : tmrHandle  --- 定时器句柄
 *               overrun  --- 定时器的超时次数，最大255
 * Output      : overrun
 * Return      : 成功时返回OS_OK，失败时返回错误码
 */
extern U32 OsSwTmrGetOverrun(TimerHandle tmrHandle, U32 *overrun);

#define OsSwtmrIqrSplLock(a) OsIntLock()
#define OsSwtmrIqrSplUnlock(a, b) OsIntRestore(b)

#endif /* PRT_SWTMR_INTERNAL_H */
