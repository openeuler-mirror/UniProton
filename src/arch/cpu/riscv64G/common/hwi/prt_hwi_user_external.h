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
 * Description: arch文档关于中断对用户需要使用的接口
 */
#ifndef PRT_HWI_USER_EXTERNAL_H
#define PRT_HWI_USER_EXTERNAL_H

extern void hwi_handler(void);          
//使用接口 - Port to user , 用户应该在发生外部中断时调用
//作用：获取当前外部中断号，并根据对应号码执行用户注册的回调函数
//使用方法： 在用户实现接口trap_entry 函数中判断是否为外部中断,若是则调用该函数


extern void hwi_timer_handler(void);   
//使用接口 - Port to user , 用户应该在发生时钟中断时调用
//作用：处理时钟tick中断的各种事务
//使用方法： 在用户实现接口trap_entry 函数中判断是否为时钟中断,若是则调用该函数

#endif