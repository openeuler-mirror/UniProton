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
 * Create: 2024-01-24
 * Description: 优先级管理链表公共头文件
 */

#ifndef PRT_PLIST_EXTERNAL_H
#define PRT_PLIST_EXTERNAL_H

#include "prt_list_external.h"

/*
 * 模块间宏定义
 */
#define OS_GET_WORD_NUM_BY_PRIONUM(prio) (((prio) + 0x1f) >> 5) //通过支持的优先级个数计算需要多少个word表示

/*
 * prioList：优先级链表（每次遇到不同的优先级链接一次）
 * nodeList：节点链表  （将所有可push的节点链接一次）
 *              |
 *              |   prio0 <--> prio2 <--------------------------> prio5 <--> prio...
 *              |     |          |          |           |           |          |
 *              |     |          |          |           |           |          |
 *              |<-->node0 <--> node1 <--> node2 <--> node3 <--> node4  <--> node...
 *              
 *              按优先级插入摘除时效率高，同优先级按FIFO顺序
 *              插入的时间复杂度o(n)，n表示优先级个数，与任务个数无关，摘除的时间复杂度为o(1)
 */
struct PushablTskList
{
    U32 prio;
    struct TagListObject nodeList;
};

/* 可push的队列链表头*/
struct PushablTskListHead
{
    struct TagListObject nodeList;
};
#endif /*PRT_PLIST_EXTERNAL_H*/
