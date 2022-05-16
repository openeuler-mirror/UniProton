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
 * Description: 队列模块的对外头文件。
 */
#ifndef PRT_QUEUE_H
#define PRT_QUEUE_H

#include "prt_module.h"
#include "prt_errno.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/*
 * 队列错误码：队列最大资源数配置成0。
 *
 * 值: 0x02000c01
 *
 * 解决方案: 队列最大资源数配置大于0，如果不用队列模块，可以配置裁剪开关为NO。
 */
#define OS_ERRNO_QUEUE_MAXNUM_ZERO OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x01)

/*
 * 队列错误码：初始化队列内存不足。
 *
 * 值: 0x02000c02
 *
 * 解决方案: 分配更大的内存分区。
 */
#define OS_ERRNO_QUEUE_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x02)

/*
 * 队列错误码：队列创建时内存不足。
 *
 * 值: 0x02000c03
 *
 * 解决方案: 可以将内存空间配大。或将创建队列的节点长度和节点个数减少。
 */
#define OS_ERRNO_QUEUE_CREATE_NO_MEMORY OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x03)

/*
 * 队列错误码：没有空闲的队列资源，已经达到配置的最大队列数。
 *
 * 值: 0x02000c04
 *
 * 解决方案: 增加配置项中队列资源数配置。
 */
#define OS_ERRNO_QUEUE_CB_UNAVAILABLE OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x04)

/*
 * 队列错误码：任务切换锁定时，禁止任务阻塞于队列。
 *
 * 值: 0x02000c05
 *
 * 解决方案: 使用前，任务先解锁。
 */
#define OS_ERRNO_QUEUE_PEND_IN_LOCK OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x05)

/*
 * 队列错误码：队列等待超时。
 *
 * 值: 0x02000c06
 *
 * 解决方案: 请查看超时时间设置是否合适。
 */
#define OS_ERRNO_QUEUE_TIMEOUT OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x06)

/*
 * 队列错误码：不能删除被任务阻塞的队列。
 *
 * 值: 0x02000c07
 *
 * 解决方案: 让阻塞的任务获得资源，不阻塞在此队列上。
 */
#define OS_ERRNO_QUEUE_IN_TSKUSE OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x07)

/*
 * 队列错误码：有超时情况下写队列不能在中断中使用。
 *
 * 值: 0x02000c08
 *
 * 解决方案: 同步队列超时时间配置为无等待或者使用异步队列。
 */
#define OS_ERRNO_QUEUE_IN_INTERRUPT OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x08)

/*
 * 队列错误码：队列未创建。
 *
 * 值: 0x02000c09
 *
 * 解决方案: 输入正确的入参。
 */
#define OS_ERRNO_QUEUE_NOT_CREATE OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x09)

/*
 * 队列错误码：阻塞在任务上的队列被激活，但没有得到调度，不能删除
 *
 * 值: 0x02000c0a
 *
 * 解决方案: 等待任务被调度后，就可以删除。
 */
#define OS_ERRNO_QUEUE_BUSY OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x0a)

/*
 * 队列错误码：队列创建时输入的指针为空。
 *
 * 值: 0x02000c0b
 *
 * 解决方案: 查看队列创建时输入的指针是否为空。
 */
#define OS_ERRNO_QUEUE_CREAT_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x0b)

/*
 * 队列错误码：队列创建时入参队列长度或者队列消息结点大小为0。
 *
 * 值: 0x02000c0c
 *
 * 解决方案: 输入正确的入参。
 */
#define OS_ERRNO_QUEUE_PARA_ZERO OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x0c)

/*
 * 队列错误码：队列句柄非法，错误或超出队列句柄范围。
 *
 * 值: 0x02000c0d
 *
 * 解决方案: 查看输入的队列句柄值是否有效。
 */
#define OS_ERRNO_QUEUE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x0d)

/*
 * 队列错误码：指针为空。
 *
 * 值: 0x02000c0e
 *
 * 解决方案: 查看输入的指针是否输入为空。
 */
#define OS_ERRNO_QUEUE_PTR_NULL OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x0e)

/*
 * 队列错误码：读写队列时buffer长度为0。
 *
 * 值: 0x02000c0f
 *
 * 解决方案: 输入正确的入参。
 */
#define OS_ERRNO_QUEUE_SIZE_ZERO OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x0f)

/*
 * 队列错误码：写队列时，输入buffer的大小大于队列结点大小。
 *
 * 值: 0x02000c10
 *
 * 解决方案: 减少buffer的大小，或者使用更大结点大小的队列。
 */
#define OS_ERRNO_QUEUE_SIZE_TOO_BIG OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x10)

/*
 * 队列错误码：读写队列时，没有资源。
 *
 * 值: 0x02000c11
 *
 * 解决方案: 写队列前需保证要有空闲的节点，读队列时需保证队列里有消息。
 */
#define OS_ERRNO_QUEUE_NO_SOURCE OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x11)

/*
 * 队列错误码：队列优先级参数有误
 *
 * 值: 0x02000c12
 *
 * 解决方案: 请检查参数，参数只能是0或1
 */
#define OS_ERRNO_QUEUE_PRIO_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x12)

/*
 * 队列错误码：节点长度超过最大值
 *
 * 值: 0x02000c13
 *
 * 解决方案: 队列节点长度不能大于0XFFFA
 */
#define OS_ERRNO_QUEUE_NSIZE_INVALID OS_ERRNO_BUILD_ERROR(OS_MID_QUEUE, 0x13)

/*
 * 队列优先级类型
 */
enum QueuePrio {
    OS_QUEUE_NORMAL = 0, /* 普通消息队列 */
    OS_QUEUE_URGENT,     /* 紧急消息队列 */
    OS_QUEUE_BUTT
};

/*
 * 队列等待时间设定：表示永久等待。
 */
#define OS_QUEUE_WAIT_FOREVER 0xFFFFFFFF

/*
 * 队列等待时间设定：表示不等待。
 */
#define OS_QUEUE_NO_WAIT 0

/*
 * 所有PID。
 */
#define OS_QUEUE_PID_ALL 0xFFFFFFFF

/*
 * @brief 创建队列。
 *
 * @par 描述
 * 创建一个队列，创建时可以设定队列长度和队列结点大小。
 * @attention
 * <ul>
 * <li>每个队列节点的大小的单位是BYTE。</li>
 * <li>每个队列节点的长度自动做2字节对齐。</li>
 * <li>每个队列节点的长度不能大于0xFFFA。</li>
 * </ul>
 * @param nodeNum     [IN]  类型#U16，队列节点个数，不能为0。
 * @param maxNodeSize [IN]  类型#U16，每个队列结点的大小。
 * @param queueId     [OUT] 类型#U32 *，存储队列ID，ID从1开始。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 * @par 依赖
 * @li prt_queue.h：该接口声明所在的头文件。
 * @see PRT_QueueDelete
 */
extern U32 PRT_QueueCreate(U16 nodeNum, U16 maxNodeSize, U32 *queueId);

/*
 * @brief 读队列。
 *
 * @par 描述
 * 读取指定队列中的数据。将读取到的数据存入bufferAddr地址，bufferAddr地址和读取数据大小由用户传入。
 * @attention
 * <ul>
 * <li>队列读取才采用FIFO模式，即先入先出，读取队列中最早写入的数据(相对于队列节点是先后顺序)。</li>
 * <li>如果bufferSize大于队列中实际消息的大小，则只返回实际大小的数据，否则只读取bufferSize大小的数据。</li>
 * <li>bufferSize大小的单位是BYTE。</li>
 * <li>阻塞模式不能在idle钩子使用，需用户保证。</li>
 * <li>在osStart之前不能调用该接口，需用户保证。</li>
 * </ul>
 * @param queueId    [IN]  类型#U32，队列ID。
 * @param bufferAddr [OUT] 类型#void *，读取存放队列中数据的起始地址。
 * @param len        [I/O] 类型#U32 *，传入BUF的大小，输出实际消息的大小。
 * @param timeOut    [IN]  类型#U32，超时时间。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * @li prt_queue.h：该接口声明所在的头文件。
 * @see PRT_QueueWrite
 */
extern U32 PRT_QueueRead(U32 queueId, void *bufferAddr, U32 *len, U32 timeOut);

/*
 * @brief 写队列。
 *
 * @par 描述
 * 向指定队列写数据。将bufferAddr地址中bufferSize大小的数据写入到队列中。
 * @attention
 * <ul>
 * <li>需保证bufferSize大小小于或等于队列结点大小。</li>
 * <li>bufferSize大小的单位是BYTE。                </li>
 * <li>阻塞模式不能在idle钩子使用，需用户保证。      </li>
 * <li>在osStart之前不能调用该接口，需用户保证。     </li>
 * </ul>
 * @param queueId    [IN]  类型#U32，队列ID。
 * @param bufferAddr [IN]  类型#void *，写到队列中数据的起始地址。
 * @param bufferSize [IN]  类型#U32，写到队列中数据的大小。
 * @param timeOut    [IN]  类型#U32，超时时间。
 * @param prio       [IN]  类型#U32，优先级, 取值OS_QUEUE_NORMAL或OS_QUEUE_URGENT。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * @li prt_queue.h：该接口声明所在的头文件。
 * @see PRT_QueueRead
 */
extern U32 PRT_QueueWrite(U32 queueId, void *bufferAddr, U32 bufferSize, U32 timeOut, U32 prio);

/*
 * @brief 删除队列。
 *
 * @par 描述
 * 删除一个消息队列。删除后队列资源被回收。
 * @attention
 * <ul>
 * <li>不能删除未创建的队列。</li>
 * <li>删除同步队列时，必须确保任务阻塞于该队列，且无被激活后还没及时操作队列的任务，否则删除队列失败。</li>
 * </ul>
 * @param queueId [IN]  类型#U32，队列ID。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * @li prt_queue.h：该接口声明所在的头文件。
 * @see PRT_QueueCreate
 */
extern U32 PRT_QueueDelete(U32 queueId);

/*
 * @brief 获取队列的历史最大使用长度。
 *
 * @par 描述
 * 获取从队列创建到删除前的历史最大使用长度。
 * @attention
 * <ul>
 * <li>峰值在队列删除前，不会被清零。</li>
 * </ul>
 * @param queueId      [IN]  类型#U32，队列ID
 * @param queueUsedNum [OUT] 类型#U32 *，队列节点使用峰值
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * @li prt_queue.h：该接口声明所在的头文件。
 * @see PRT_QueueGetNodeNum
 */
extern U32 PRT_QueueGetUsedPeak(U32 queueId, U32 *queueUsedNum);

/*
 * @brief 获取指定源PID的待处理消息个数。
 *
 * @par 描述
 * 从指定队列中，获取指定源PID的待处理消息个数。
 * @attention
 * <ul>
 * <li>PID为OS_QUEUE_PID_ALL时，表示获取所有待处理的消息个数 </li>
 * <li>PID的合法性不做判断，不合法的PID获取的消息个数为0     </li>
 * </ul>
 * @param queueId  [IN]  类型#U32，队列ID。
 * @param taskPid  [IN]  类型#U32，线程PID。
 * @param queueNum [OUT] 类型#U32 *，待处理的消息个数。
 *
 * @retval #OS_OK  0x00000000，操作成功。
 * @retval #其它值，操作失败。
 *
 * @par 依赖
 * @li prt_queue.h：该接口声明所在的头文件。
 * @see PRT_QueueGetUsedPeak
 */
extern U32 PRT_QueueGetNodeNum(U32 queueId, U32 taskPid, U32 *queueNum);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif /* PRT_QUEUE_H */
