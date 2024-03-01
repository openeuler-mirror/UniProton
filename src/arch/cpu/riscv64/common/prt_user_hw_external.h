/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-1-18
 * Description: arch文档关于对用户需要使用的接口 或者实现的接口 
 * 这个头文件是描述在异常，中断，陷入时需要调用的函数
 */
#ifndef PRT_USER_HW_EXTERNAL_H
#define PRT_USER_HW_EXTERNAL_H

//关于异常给用户使用的分发入口函数在下面头文件
#include "./exc/prt_user_exc_external.h"

//关于中断给用户使用的分发入口函数在下面头文件
#include "./prt_user_exc_external.h"

//下面是关于陷入时候用户使用或者需要实现的函数
extern void trap(void); 
//使用接口 - Port to user , 用户应该在初始化时将mtvec的值写为trap的地址
//作用：     陷入执行的入口地址(外部中断 异常 tick中断执行的入口)
//          会进行上文保存，若不是嵌套中断，将上文保存到线程的栈中，若是嵌套中断，嵌套在系统栈中
//使用方法： 初始化的时候将mtvec的值写为trap
//注意事项:  在trap入口保存上文后就会对系统的状态设置 g_uniFlag 不用再在中断处理中进行设置

extern void trap_entry(U64 mcause); 
//实现接口 - Port to user, 用户应该实现这个接口，mcause 为发生陷入的mcause值
//作用：     用户根据mcause 判断陷入类型，中断则调用中断处理的接口，tick 同理


#endif