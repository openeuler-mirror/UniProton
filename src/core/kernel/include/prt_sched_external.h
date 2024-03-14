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
 * Description: 调度的公共函数实现私有头文件
 */
#ifndef PRT_SCHED_EXTERNAL_H
#define PRT_SCHED_EXTERNAL_H

#include "prt_lib_external.h"
#include "prt_raw_spinlock_external.h"
#include "prt_err_external.h"
#include "prt_sys_external.h"
#include "prt_cpu_external.h"
#include "prt_rt_external.h"
#include "prt_atomic.h"
#include "prt_typedef.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#if defined(OS_OPTION_SMP)
/*
 *模块间宏定义
 */
#define SMP_MC_SCHEDULE_TRIGGER(core) OsHwiTriggerByMask((U32)(1UL << (core)), OS_SMP_SCHED_TRIGGER_OTHER_CORE_SGI)
#define SMP_MC_SCHEDULE_TRIGGER_SELF(core) OsHwiTriggerSelf(core, OS_HWI_IPI_NO_01)

/* 根据核号取对应的rq*/
#define GET_RUNQ(core) ((struct TagOsRunQue *)&g_runQueue[(core)])
#define OS_RT_MAX_TRYS 3

#define THIS_RUNQ() ((struct TagOsRunQue *)&g_runQueue[THIS_CORE()])

#define OS_SCHED_CLASS(coreID)      (GET_RUNQ(coreID)->schedClass)

#define RT_SINGLE_CLASS()      (&g_osRtSingleSchedClass)

enum OsScheduleType{
    OS_SCHEDULE_RT_SINGLE //单硬线程调度
};

#define OS_DEFAULT_SCHED_TYPE OS_SCHEDULE_RT_SINGLE

/* 入队出队的flags */
#define OS_ENQUEUE_HEAD 1U /* 入队加头 */

/*
 * 模块间结构体定义
 */
struct TagTskCb;
struct TagScheduleClass;

/*
 * 每个核一份的running队列
 * spinlock/uniFlag/tskCurr 这几个变量位置不能动，汇编代码会使用
 */
struct TagOsRunQue {
    volatile uintptr_t spinLock;        // 操作该RQ使用的锁
    U32 uniFlag;
    struct TagTskCb *tskCurr;           // RQ中的当前运行任务
    bool needReschedule;                // 是否需要调度
    U32 rqCoreId;                       // 运行队列所属的核
    U32 tskIdlePID;                     // RQ中的idle任务
    U32 nrRunning;                      // RQ中总运行任务个数
    U32 intCount;                       // 中断进入次数
    U32 tickNoRespondCnt;               // tick待响应次数
    U16 uniTaskLock;                    // 锁任务计数
    U16 reserved;
    U32 shakeCount;                     // 核间握手计数
    bool online;                        // 队列是否还在线
    U32 currntPrio;                     // RQ中最高优先级任务的优先级
    struct RtRq rtRq;                   // 实时优先级运行队列
    struct TagScheduleClass *schedClass; //调度方法
};
/*
 * 模块间全局变量
 */
extern struct TagOsRunQue g_runQueue[OS_MAX_CORE_NUM];

#define OS_SMP_HANDSHAKE_COUNT(core) (GET_RUNQ(core)->shakeCount)

/*
 * 模块间函数定义
 */
extern void OsReschedTask(struct TagTskCb *task);
extern void OsReschedTaskNoWakeIpc(struct TagTskCb *task);
extern void OsSmpSendReschedule(U32 coreID);

extern U32 g_tskMaxNum;

/* WARNING: 下面的几个函数一定都是在关中断下进行的，关中断由外部保证 */
OS_SEC_ALW_INLINE INLINE void OsDoubleRqLock(struct TagOsRunQue *thisRq, struct TagOsRunQue *busiestRq)
{
    if (thisRq < busiestRq) {
        OsSplLock(&thisRq->spinLock);
        OsSplLock(&busiestRq->spinLock);
    } else if (thisRq > busiestRq) {
        OsSplLock(&busiestRq->spinLock);
        OsSplLock(&thisRq->spinLock);
    } else {
        // equal
        OsSplLock(&thisRq->spinLock);
    }
    return;
}

/* 避免死锁，锁住两个rq */
OS_SEC_ALW_INLINE INLINE void OsDoubleLockBalance(struct TagOsRunQue *thisRq, struct TagOsRunQue *busiestRq)
{
    OsSplUnlock(&thisRq->spinLock);
    OsDoubleRqLock(thisRq, busiestRq);
}

/* double解锁 */
OS_SEC_ALW_INLINE INLINE void OsDoubleUnlockBalance(struct TagOsRunQue *thisRq, struct TagOsRunQue *busiestRq)
{
    OsSplUnlock(&busiestRq->spinLock);
    if (thisRq != busiestRq) {
        OsSplUnlock(&thisRq->spinLock);
    }
}

/* double解锁，只解destRq */
OS_SEC_ALW_INLINE INLINE void OsDoubleUnlockDest(struct TagOsRunQue *thisRq, struct TagOsRunQue *destRq)
{
    if (thisRq != destRq) {
        OsSplUnlock(&destRq->spinLock);
    }
}

#define CPU_AGENT_CPU(cpu)   (cpu)
OS_SEC_ALW_INLINE INLINE void OsWorkHandler(void) {}

extern void OsContextSwitch(struct TagTskCb *prev, struct TagTskCb *next);
extern void OsDeactiveTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags);
extern void OsActiveTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags);
extern struct TagTskCb *OsPickNextTask(struct TagOsRunQue *runQue);

extern void OsSwitchTgrpSpace(uintptr_t mmuTbl, U32 contextId);
extern bool OsTskSchedNextRtTask(U32 core, TskHandle *tid, U32* coreId);
extern void OsDequeueTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 sleep);
extern void OsEnqueueTask(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags);

// 后续用钩子
extern void OsMainSchedule(void);

OS_SEC_ALW_INLINE INLINE void OsRescheduleCurr(struct TagOsRunQue *runQue)
{
    runQue->needReschedule = TRUE;
}

/* 将运行队列的任务计数++ */
OS_SEC_ALW_INLINE INLINE void OsIncNrRunning(struct TagOsRunQue *runQue)
{
    runQue->nrRunning++;
}

OS_SEC_ALW_INLINE INLINE void OsDecNrRunning(struct TagOsRunQue *runQue)
{
    runQue->nrRunning--;
}

OS_SEC_ALW_INLINE INLINE bool OsSchedIsDomainTypeValid(enum OsScheduleType type)
{
    return (type == OS_SCHEDULE_RT_SINGLE);
}

extern struct TagScheduleClass *OsGetRtSingleSchedClass(void);

OS_SEC_ALW_INLINE INLINE struct TagScheduleClass *OsSchedGetSchedClass()
{
    return OsGetRtSingleSchedClass();
}

#endif
#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cpluscplus */
#endif /* __cpluscplus */

#endif