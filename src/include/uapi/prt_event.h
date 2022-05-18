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
 * Description: 事件模块的对外头文件。
 */
#ifndef PRT_EVENT_H
#define PRT_EVENT_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 事件等待时间设定：事件最大等待超时值设置。
 */
#define OS_EVENT_WAIT_FOREVER 0xFFFFFFFF

/*
 * 事件读取模式：表示期望eventMask中的任何一个事件。
 */
#define OS_EVENT_ANY 0x00000001

/*
 * 事件读取模式：表示期望接收eventMask中的所有事件。
 */
#define OS_EVENT_ALL 0x00000010

/*
 * 事件读取模式：表示等待接收事件。
 */
#define OS_EVENT_WAIT 0x00010000

/*
 * 事件读取模式：表示不等待接收事件。
 */
#define OS_EVENT_NOWAIT 0x00100000

/*
 * 事件错误码：事件读取失败，期望事件没有发生。
 *
 * 值: 0x02000b01
 *
 * 解决方案：可使用等待读取事件。
 */
#define OS_ERRNO_EVENT_READ_FAILED OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x01)

/*
 * 事件错误码：读取事件超时。
 *
 * 值: 0x02000b02
 *
 * 解决方案：增大事件读取等待时间，或其他任务给该任务写事件操作。
 */
#define OS_ERRNO_EVENT_READ_TIMEOUT OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x02)

/*
 * 事件错误码：写事件时入参任务ID非法。
 *
 * 值: 0x02000b03
 *
 * 解决方案: 请输入合法任务ID。
 */
#define OS_ERRNO_EVENT_TASKID_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x03)

/*
 * 事件错误码：写事件时入参任务未创建。
 *
 * 值: 0x02000b04
 *
 * 解决方案: 请输入合法任务ID，或先创建任务再对其写事件操作。
 */
#define OS_ERRNO_EVENT_TSK_NOT_CREATED OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x04)

/*
 * 事件错误码：读事件时EVENTMASK入参非法，入参不能为0。
 *
 * 值: 0x02000b05
 *
 * 解决方案: 请输入合法值。
 */
#define OS_ERRNO_EVENT_EVENTMASK_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x05)

/*
 * 事件错误码：只能在任务中进行读事件操作。
 *
 * 值: 0x02000b06
 *
 * 解决方案: 请在任务中进行读事件操作。
 */
#define OS_ERRNO_EVENT_READ_NOT_IN_TASK OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x06)

/*
 * 事件错误码
 * 读事件接口中flags入参非法，该入参为（OS_EVENT_ANY，OS_EVENT_ALL）中一个和
 * （OS_EVENT_WAIT，OS_EVENT_NOWAIT）中的一个标识或的结果;
 * OS_EVENT_WAIT模式下，等待时间必须非零。
 *
 * 值: 0x02000b07
 *
 * 解决方案: 请输入合法的入参。
 */
#define OS_ERRNO_EVENT_FLAGS_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x07)

/*
 * 事件错误码：在锁任务调度状态下，禁止任务阻塞于读事件。
 *
 * 值: 0x02000b08
 *
 * 解决方案: 请解锁任务调度后，再进行读事件。
 */
#define OS_ERRNO_EVENT_READ_IN_LOCK OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x08)

/*
 * 事件错误码：写事件时EVENT入参非法，入参不能为0。
 *
 * 值: 0x02000b09
 *
 * 解决方案: 请输入合法值。
 */
#define OS_ERRNO_EVENT_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_EVENT, 0x09)

/*
 * @brief 读事件。
 *
 * @par 描述
 * 读取当前任务的指定掩码为eventMask的事件，可以一次性读取多个事件。事件读取成功后，被读取的事件将被清除。
 * @attention
 * <ul>
 * <li>读取模式可以选择读取任意事件和读取所有事件。</li>
 * <li>等待模式可以选择等待和不等待，可等待指定时间、永久等待。</li>
 * <li>当读取事件没有发生且为等待模式时，会发生任务调度，锁任务状态除外。</li>
 * <li>不能在IDLE任务中读事件，需要用户自行保证。</li>
 * <li>不能在系统初始化之前调用读事件接口。</li>
 * </ul>
 *
 * @param eventMask [IN]  类型#U32，设置要读取的事件掩码，每个bit位对应一个事件，1表示要读取。该入参不能为0。
 * @param flags     [IN]  类型#U32，读取事件所采取的策略, 为（OS_EVENT_ANY，OS_EVENT_ALL）中一个和（OS_EVENT_WAIT，
 * OS_EVENT_NOWAIT）中的一个标识或的结果。#OS_EVENT_ALL表示期望接收eventMask中的所有事件，
 * #OS_EVENT_ANY表示等待eventMask中的任何一个事件。#OS_EVENT_WAIT表示若期望事件没有发生，等待接收，
 * #OS_EVENT_NOWAIT表示若期望事件没有发生，将不会等待接收。
 * @param timeOut   [IN]  类型#U32，等待超时时间，单位为tick，取值(0~0xFFFFFFFF]。当flags标志为OS_EVENT_WAIT，
 * 这个参数才有效。若值为#OS_EVENT_WAIT_FOREVER，则表示永久等待。
 * @param events    [OUT] 类型#U32 *，用于保存接收到的事件的指针。如果不需要输出，可以填写NULL。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * <ul>
 * @li prt_event.h：该接口声明所在的头文件。
 * </ul>
 * @see PRT_EventWrite
 */
extern U32 PRT_EventRead(U32 eventMask, U32 flags, U32 timeOut, U32 *events);

/*
 * @brief 写事件。
 *
 * @par 描述
 * 写任务ID为taskId的指定事件，可以一次性写多个事件，可以在UniProton接管的中断中调用。
 * @attention
 * <ul>
 * <li>若指定任务正在等待读取写入的事件，则会激活指定任务，并发生任务调度。</li>
 * <li>不能向IDLE任务写事件，需要用户自行保证。</li>
 * </ul>
 *
 * @param taskId [IN]  类型#U32，任务ID，表示要对某个任务进行写事件操作。
 * @param events [IN]  类型#U32，事件号，每个bit对应一个事件。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * <ul>
 * @li prt_event.h：该接口声明所在的头文件。
 * </ul>
 * @see PRT_EventRead
 */
extern U32 PRT_EventWrite(U32 taskId, U32 events);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_EVENT_H */
