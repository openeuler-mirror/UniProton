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
 * Description: arch文档关于异常对用户需要使用的接口
 */
#ifndef PRT_USER_EXC_EXTERNAL_H
#define PRT_USER_EXC_EXTERNAL_H

extern void OsExcDispatch();    
//使用接口 - Port to user , 用户应该在发生异常时刻调用
//作用：保存当前各种异常信息到内部数据结构，并执行用户的hook函数
//使用方法： 用户可以注册hook函数进行打印，实现异常dump

#endif
