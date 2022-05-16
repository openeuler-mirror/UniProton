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
 * Description: CPU占用率模块对外头文件。
 */
#ifndef PRT_CPUP_H
#define PRT_CPUP_H

#include "prt_buildef.h"
#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * CPUP错误码：参数(CPUP告警阈值)设定不在规定范围(0,10000]。
 *
 * 值: 0x02000601
 *
 * 解决方案: CPUP告警阈值设定在规定范围(0,10000]内。
 */
#define OS_ERRNO_CPUP_INTERVAL_NOT_SUITED OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x01)

/*
 * CPUP错误码：指针参数为NULL。
 *
 * 值: 0x02000602
 *
 * 解决方案: 传入非0的有效地址。
 */
#define OS_ERRNO_CPUP_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x02)

/*
 * CPUP错误码：恢复阈值设定不小于告警阈值。
 *
 * 值: 0x02000603
 *
 * 解决方案: 恢复阈值小于告警阈值。
 */
#define OS_ERRNO_CPUP_RESUME_NOT_SUITED OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x03)

/*
 * CPUP错误码：CPUP初始化申请内存失败。
 *
 * 值: 0x02000604
 *
 * 解决方案: 确认缺省静态内存是否足够，以及采样个数(参见配置宏OS_CPUP_SAMPLE_RECORD_NUM)是否过大。
 */
#define OS_ERRNO_CPUP_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x04)

/*
 * CPUP错误码：在osStart之前调用CPUP模块相关功能接口。
 *
 * 值: 0x02000605
 *
 * 解决方案: 调用CPUP模块功能接口时，请查看是否在osStart之后。
 */
#define OS_ERRNO_CPUP_OS_NOT_STARTED OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x05)

/*
 * CPUP错误码：CPUP采样时间间隔为0。
 *
 * 值: 0x02000606
 *
 * 解决方案: 系统级CPUP设置采样时间间隔必须大于0，当CPUP告警功能打开时，线程级CPUP采样时间间隔也必须大于0。
 */
#define OS_ERRNO_CPUP_SAMPLE_TIME_ZERO OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x06)

/*
 * CPUP错误码：CPUP功能开关未打开或者未初始化时，获取CPUP、设置告警阈值或者补偿IDLE钩子执行时间。
 *
 * 值: 0x02000607
 *
 * 解决方案: 需保证在打开功能开关,在osStart之后才能获取CPUP、设置告警阈值或者补偿IDLE钩子执行时间。
 */
#define OS_ERRNO_CPUP_NOT_INITED OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x07)

/*
 * CPUP错误码：使用获取线程级所有线程的CPUP接口时，输入的线程数为0。
 *
 * 值: 0x02000608
 *
 * 解决方案: 使用获取线程级所有线程的CPUP接口时，输入的线程数必须大于0
 */
#define OS_ERRNO_CPUP_THREAD_INNUM_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x08)

/*
 * CPUP错误码：CPUP告警恢复及CPUP告警阈值配置错误。
 *
 * 值: 0x02000609
 *
 * 解决方案: CPUP告警阈值设定应该小于CPUP告警恢复阈值。
 */
#define OS_ERRNO_CPUP_RESUME_VALUE_ERROR OS_ERRNO_BUILD_ERROR(OS_MID_CPUP, 0x09)

/*
 * CPU占用率告警标志。
 */
#define CPUP_INFO_TYPE_OVERLOAD 0x01

/*
 * CPU占用率恢复告警标志。
 */
#define CPUP_INFO_TYPE_RECONVERT 0x02

/*
 * 设置cpu占用率的注册信息结构体。
 */
struct CpupModInfo {
    /* CPUP告警标志 */
    bool cpupWarnFlag;
    /* 采样间隔，单位tick */
    U32 sampleTime;
    /* CPU占用率告警阈值 */
    U32 warn;
    /* CPU占用率告警恢复阈值 */
    U32 resume;
};

/*
 * CPUP告警信息。
 */
struct CpupWarnInfo {
    /* CPU占用率告警信息类型 */
    U16 type;
    /* 保留 */
    U16 reserve;
};

/*
 * @brief CPUP告警回调函数类型定义。
 *
 * @par 描述
 * 通过该回调函数的类型定义回调函数钩子。
 * @attention 无
 *
 * @param  #struct CpupWarnInfo*   [IN] 类型#struct CpupWarnInfo*，CPUP告警信息。
 *
 * @retval 无。
 * @par 依赖
 * <ul><li>prt_cpup.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
typedef void (*CpupHookFunc)(struct CpupWarnInfo *);

/*
 * 线程级CPU占用率结构体。
 */
struct CpupThread {
    /* 线程ID */
    U32 id;
    /* 占用率，取值[0,10000] */
    U16 usage;
    /* 保留 */
    U16 resv;
};

/*
 * @brief 获取当前cpu占用率。
 *
 * @par 描述
 * 通过本接口获取当前cpu占用率。
 * @attention
 * <ul>
 * <li>该接口必须在CPUP模块裁剪开关打开，并在osStart之后才能调用此接口，否则返回0xffffffff。</li>
 * <li>精度为万分之一。</li>
 * <li>为了减小CPUP统计对线程调度的性能影响，OS采用了基于IDLE计数的统计算法，
 * 统计结果会有一定误差，误差不超过百分之五。</li>
 * </ul>
 *
 * @param 无。
 *
 * @retval #0xffffffff      获取失败，CPUP裁剪开关未打开，或未初始化，或者在osStart之前调用。
 * @retval #[0,10000]       返回当前cpu占用率。
 * @par 依赖
 * <ul><li>prt_cpup.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_CpupAvg | PRT_CpupThread
 */
extern U32 PRT_CpupNow(void);

/*
 * @brief 获取指定个数的线程的CPU占用率。
 *
 * @par 描述
 * 根据用户输入的线程个数，获取指定个数的线程CPU占用率。
 * @attention
 * <ul>
 * <li>当且仅当CPUP模式配置为线程级时，该接口有效。</li>
 * <li>当配置项中的采样周期值等于0时，线程级CPUP采样周期为两次调用该接口或者PRT_CpupNow之间
 * 的间隔。否则，线程级CPUP采样周期为配置项中的OS_CPUP_SAMPLE_INTERVAL大小。</li>
 * <li>输出的实际线程个数不大于系统中实际的线程个数(任务个数和一个中断线程)。</li>
 * <li>若输入的线程个数为1，则仅输出中断线程(除任务线程以外的线程统称)的CPUP信息。</li>
 * <li>若在一个采样周期内有任务被删除，则统计的任务线程和中断线程CPUP总和小于10000。</li>
 * </ul>
 *
 * @param inNum  [IN]  类型#U32，输入的线程个数。
 * @param cpup   [OUT] 类型#struct CpupThread *，缓冲区，输出参数，用于填写输出个数线程的CPUP信息。
 * @param outNum [OUT] 类型#U32 *，保存输出的实际线程个数指针。
 *
 * @retval #OS_OK  0x00000000，获取成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_cpup.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_CpupNow
 */
extern U32 PRT_CpupThread(U32 inNum, struct CpupThread *cpup, U32 *outNum);

#if defined(OS_OPTION_CPUP_WARN)
/*
 * @brief 设置CPU占用率告警阈值。
 *
 * @par 描述
 * 根据用户配置的CPU占用率告警阈值warn和告警恢复阈值resume，设置告警和恢复阈值。
 * @attention
 * <ul>
 * <li>OsStart之前不能调用此接口。</li>
 * </ul>
 *
 * @param warn   [IN]  类型#U32，CPUP告警阈值。
 * @param resume [IN]  类型#U32，CPUP恢复阈值。
 *
 * @retval #OS_OK  0x00000000，阈值设定成功。
 * @retval #其它值，阈值设定失败。
 * @par 依赖
 * <ul><li>prt_cpup.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_CpupGetWarnValue
 */
extern U32 PRT_CpupSetWarnValue(U32 warn, U32 resume);

/*
 * @brief 查询CPUP告警阈值和告警恢复阈值
 *
 * @par 描述
 * 根据用户配置的告警阈值指针warn和告警恢复阈值指针resume，查询告警阈值和告警恢复阈值
 * @attention
 * <ul>
 * <li>OsStart之前不能调用此接口。</li>
 * </ul>
 *
 * @param warn   [OUT] 类型#U32 *，CPUP告警阈值。
 * @param resume [OUT] 类型#U32 *，CPUP恢复阈值。
 *
 * @retval #OS_OK  0x00000000，获取成功。
 * @retval #其它值，获取失败。
 * @par 依赖
 * <ul><li>prt_cpup.h：该接口声明所在的头文件。</li></ul>
 * @see PRT_CpupSetWarnValue
 */
extern U32 PRT_CpupGetWarnValue(U32 *warn, U32 *resume);

/*
 * @brief 注册CPUP告警回调函数
 *
 * @par 描述
 * 根据用户配置的回调函数hook，注册CPUP告警回调函数
 * @attention
 * <ul>
 * <li>不允许重复或覆盖注册钩子。</li>
 * <li>hook为NULL时，表示删除该钩子。</li>
 * </ul>
 *
 * @param hook [IN]  类型#CpupHookFunc，CPU告警回调函数。
 *
 * @retval #OS_OK  0x00000000，注册成功。
 * @retval #其它值，注册失败。
 * @par 依赖
 * <ul><li>prt_cpup.h：该接口声明所在的头文件。</li></ul>
 * @see 无。
 */
extern U32 PRT_CpupRegWarnHook(CpupHookFunc hook);
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_CPUP_H */
