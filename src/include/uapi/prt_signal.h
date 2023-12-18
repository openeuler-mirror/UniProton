/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-08
 * Description: 信号模块对外头文件。
 */
#ifndef PRT_SIGNAL_H
#define PRT_SIGNAL_H

#include "prt_typedef.h"
#include "prt_task.h"
#include "bits/list_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#define PRT_SIGNAL_MAX 32
/* 支持编号为1-31的信号 */
#define sigMask(signo) (1U << (signo - 1))
#define sigValid(signo) (signo > 0 && signo < PRT_SIGNAL_MAX)
/* 每bit表示一种信号 */
typedef unsigned long signalSet;

typedef void (*_sa_handler)(int);

struct _sigaction {
    _sa_handler saHandler;
};

typedef struct {
    int si_signo;
    int si_code;
} signalInfo;

typedef struct {
    signalInfo siginfo;
    struct TagListObject siglist;
} sigInfoNode;

/*
 * 信号等待时间设定：信号最大等待超时值设置。
 */
#define OS_SIGNAL_WAIT_FOREVER 0xFFFFFFFF

/*
 * 信号错误码：函数入参错误。
 *
 * 值: 0x02001001
 *
 * 解决方案：传入正确的函数参数。
 */
#define OS_ERRNO_SIGNAL_PARA_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x01)

/*
 * 信号错误码：信号等待超时。
 *
 * 值: 0x02001002
 *
 * 解决方案：增大信号等待时间，或者给该任务尽早发送信号。
 */
#define OS_ERRNO_SIGNAL_TIMEOUT OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x02)

/*
 * 信号错误码：只能在任务中等待信号。
 *
 * 值: 0x02001003
 *
 * 解决方案：请在任务中等待信号。
 */
#define OS_ERRNO_SIGNAL_WAIT_NOT_IN_TASK OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x03)

/*
 * 信号错误码：在锁任务调度状态下，禁止任务阻塞于信号等待。
 *
 * 值: 0x02001004
 *
 * 解决方案: 请解锁任务调度后，再进行信号等待。
 */
#define OS_ERRNO_SIGNAL_WAIT_IN_LOCK OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x04)

/*
 * 信号错误码：信号处理函数中，taskId入参不合法。
 *
 * 值: 0x02001005
 *
 * 解决方案: 请传入正确的taskId。
 */
#define OS_ERRNO_SIGNAL_TASKID_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x05)

/*
 * 信号错误码：信号处理函数中，对应task未创建。
 *
 * 值: 0x02001006
 *
 * 解决方案: 请传入正确的taskId，或先创建任务再对其传入信号。
 */
#define OS_ERRNO_SIGNAL_TSK_NOT_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x06)

/*
 * 信号错误码：信号处理函数中，信号值入参不合法。
 *
 * 值: 0x02001007
 *
 * 解决方案: 请传入正确的信号值。
 */
#define OS_ERRNO_SIGNAL_NUM_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x07)

/*
 * 信号错误码：信号处理函数中，申请内存不足。
 *
 * 值: 0x02001008
 *
 * 解决方案: 为信号处理模块分配更多的内存。
 */
#define OS_ERRNO_SIGNAL_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_SIGNAL, 0x08)

/*
 * @brief 信号处理。
 *
 * @par 描述
 * 处理发送给指定任务的信号。
 * @attention
 * <ul>
 * <li>判断对应的任务是否在等待该信号，若是则将该任务加入就绪队列，触发一次调度。</li>
 * <li>判断信号是否为任务阻塞信号，若是则加入未决信号，返回不处理。</li>
 * <li>判断信号是否由当前正在运行的任务处理，若是则直接处理。</li>
 * <li>判断信号是否由其他就绪的任务处理，若是则将该任务的上下文保存并使下次调度直接进入信号处理函数，并尝试进行任务调度。</li>
 * </ul>
 *
 * @param taskId  [IN]  类型#TskHandle，任务ID。
 * @param info    [IN]  类型#signalInfo*，等到的信号信息。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul>
 * @li prt_signal.h：该接口声明所在的头文件。
 * </ul>
 * @see PRT_SignalWait
 */
extern U32 PRT_SignalDeliver(TskHandle taskId, signalInfo *info);

/*
 * @brief 等待信号。
 *
 * @par 描述
 * 使任务同步等待指定信号集中的信号，并返回等到信号的信息。
 * @attention
 * <ul>
 * <li>若指定等待的信号中不存在未决信号，则任务进行挂起等待，若存在未决信号则返回其中一个未决信号的信息，并清除未决标记。</li>
 * <li>若指定了timeOutTick，任务会等待指定时间，若timeOutTick入参为OS_SIGNAL_WAIT_FOREVER，任务会永久等待。</li>
 * </ul>
 *
 * @param set         [IN]   类型#signalSet*，等待信号集，表示要等待哪些信号。
 * @param info        [OUT]  类型#signalSet*，等到的信号信息。
 * @param timeOutTick [IN]   类型#U32，等待信号的时间(Tick)。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul>
 * @li prt_signal.h：该接口声明所在的头文件。
 * </ul>
 * @see PRT_SignalDeliver
 */
extern U32 PRT_SignalWait(const signalSet *set, signalInfo *info, U32 timeOutTick);

/*
 * @brief 设置信号集状态并等待。
 *
 * @par 描述
 * 设置信号集屏蔽状态并等待。
 * @attention
 * <ul>
 * <li>设置信号集状态，若有信号发生并执行信号处理返回</li>
 * </ul>
 *
 * @param set         [IN]   类型#signalSet*，屏蔽信号集。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul>
 * @li prt_signal.h：该接口声明所在的头文件。
 * </ul>
 * @see PRT_Sigsuspend
 */
extern U32 PRT_SigSuspend(const signalSet *mask);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /* PRT_SIGNAL_H */
