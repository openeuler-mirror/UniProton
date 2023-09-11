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
 * Create: 2023-08-12
 * Description: KAL IPC头文件。
 */
#ifndef KAL_IPC_H
#define KAL_IPC_H

#ifndef _GNU_SOURCE
#define _GNU_SOURCE 1
#endif

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <time.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cpluscplus */
#endif /* __cpluscplus */

/**
 * @brief 查找消息队列
 * @param msgid [IN] 需要查找的消息队列id
 * 
 * @retval 0为成功
 * */
int KAL_MsgFind(uintptr_t msgid);

/**
 * @brief 创建消息队列
 * @param flag [IN] 消息队列创建的模式标识
 * @param mode [IN] 消息队列创建的权限
 * @param maxnum [IN] 消息队列的最大消息数量
 * @param maxsize [IN] 消息队列的最大消息大小
 * @param msgid [OUT] 返回的消息队列id
 * 
 * @retval 0为成功
 * */
int KAL_MsgCreate(int flag, int mode, int maxnum, int maxsize, uintptr_t *msgid);

/**
 * @brief 获取消息队列信息
 * @param msgid [IN] 消息队列id
 * @param buf [OUT] 返回的msqid_ds信息
 * 
 * @retval 0为成功
 * */
int KAL_MsgCtlStat(uintptr_t msgid, struct msqid_ds *buf);

/**
 * @brief 删除消息队列
 * @param msgid [IN] 消息队列id
 * 
 * @retval 0为成功
 * */
int KAL_MsgDelete(uintptr_t msgid);

/**
 * @brief 发送消息
 * @param msgid [IN] 消息队列id
 * @param msgp [IN] 需要发送的消息
 * @param msgsz [IN] 需要发送的消息的大小
 * @param flag [IN] 发送消息的模式标识
 * @param timeout [IN] 发送消息的超时时间
 * 
 * @retval 0为成功
 * */
int KAL_MsgSend(uintptr_t msgid, const void *msgp, size_t msgsz, int flag, const struct timespec *timeout);

/**
 * @brief 接受消息
 * @param msgid [IN] 消息队列id
 * @param msgp [IN] 需要接受的消息
 * @param msgp [IN/OUT] 接受消息的大小
 * @param msgtype [IN] 控制接受消息的类型
 * @param flag [IN] 接受消息的模式标识
 * @param timeout [IN] 接受消息的超时时间
 * 
 * @retval 0为成功
 * */
int KAL_MsgRecv(uintptr_t msgid, void *msgp, ssize_t *msgsz, long msgtype, int flag, const struct timespec *timeout);

/**
 * @brief 创建单个信号量
 * @param flag [IN] 创建信号量的模式标识
 * @param mode [IN] 创建信号量的权限
 * @param value [IN] 创建信号量的初始值
 * @param semid [OUT] 新建的信号量id
 * 
 * @retval 0为成功
 * */
int KAL_SemCreate(int flag, int mode, int value, uintptr_t *semid);

/**
 * @brief 删除单个信号量
 * @param semid [IN] 信号量id
 * 
 * @retval 0为成功
 * */
int KAL_SemDelete(uintptr_t semid);

/**
 * @brief 释放单个信号量
 * @param semid [IN] 信号量id
 * @param value [IN] 释放的资源数量，通常为1
 * @param flag [IN] 操作模式，#IPC_NOWAIT表示不等待，#SEM_UNDO支持撤销操作
 * 
 * @retval 0为成功
 * */
int KAL_SemPost(uintptr_t semid, int value, int flag);

/**
 * @brief 等待单个信号量
 * @param semid [IN] 信号量id
 * @param value [IN] 申请的资源数量，通常为1
 * @param flag [IN] 操作模式，#IPC_NOWAIT表示不等待，#SEM_UNDO支持撤销操作
 * @param timeout [IN] 设置等待时间
 * 
 * @retval 0为成功
 * */
int KAL_SemWait(uintptr_t semid, int value, int flag, const struct timespec *timeout);

/**
 * @brief 获取单个信号量的值
 * @param semid [IN] 信号量id
 * @param value [IN] 获取的值
 * 
 * @retval 0为成功
 * */
int KAL_SemGetValue(uintptr_t semid, int *value);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif