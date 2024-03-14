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
 * Description: 实时调度公共头文件
 */
#ifndef PRT_RT_EXTERNAL_H
#define PRT_RT_EXTERNAL_H

#include "prt_plist_external.h"
#include "prt_task_external.h"

/*
 * 模块间宏定义
 */
// pendList复用作为就绪链表节点
#define GET_TCB_READY(ptr) LIST_COMPONENT(ptr, struct TagTskCb, pendList)

/*
 * 模块间结构体定义
 */
/*
 * 就绪队列链表
 */
struct RtActiveTskList
{
    /* 优先级bit位表 */
    U32 readyPrioBit[OS_GET_WORD_NUM_BY_PRIONUM(OS_TSK_NUM_OF_PRIORITIES)];
    struct TagListObject readyList[OS_TSK_NUM_OF_PRIORITIES];
};

/*
 * 实时运行队列
 */
struct RtRq {
    /* 该队列是否overload*/
    bool isOverload;
    /* pushable链表中最高的优先级 */
    U32 nextPrio;
    /* 运行队列上任务的个数 */
    U32 nrRunning;
    /* ready链表 */
    struct RtActiveTskList activeTsk;
    /* 可push链表 */
    struct PushablTskListHead pushAblList;
};

struct TagOsRunQue;
struct TagTskCb;

extern void OsEnqueueTaskRtSingle(struct TagOsRunQue *runQue, struct TagTskCb *task, U32 flags);
extern void OsDequeueTaskRtSingle(struct TagOsRunQue *runQue, struct TagTskCb *task, U32 flags);
extern void OsPutPrevTaskRtSingle(struct TagOsRunQue *runQue, struct TagTskCb *prevTskCB);
extern struct TagTskCb *OsPickNextTaskRtSingle(struct TagOsRunQue* runQue);
extern struct TagTskCb *OsNextReadyRtTaskSingle(struct TagOsRunQue *runQue);
#endif