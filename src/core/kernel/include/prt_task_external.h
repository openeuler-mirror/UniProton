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
 * Description: 任务模块的内部头文件
 */
#ifndef PRT_TASK_EXTERNAL_H
#define PRT_TASK_EXTERNAL_H

#include "prt_task.h"
#include "prt_lib_external.h"
#include "prt_list_external.h"
#include "prt_sys_external.h"
#include "prt_err_external.h"
#include "prt_tick_external.h"
#include "prt_cpu_external.h"
#include "prt_mem_external.h"
#include "prt_atomic.h"
#include "prt_plist_external.h"
#include "prt_sched_external.h"

#if defined(OS_OPTION_NUTTX_VFS)
#include "nuttx/fs/fs.h"
#if defined(CONFIG_FILE_STREAM)
#include "nuttx/tls.h"
#endif
#endif

extern U8 g_maxNumOfCores;
#define OS_COREID_CHK_INVALID(coreId) ((coreId) >= g_maxNumOfCores)

#if defined(OS_OPTION_POSIX)
#include "pthread.h"
#ifndef PTHREAD_KEYS_MAX
#define PTHREAD_KEYS_MAX 32
#endif
#if defined(OS_OPTION_POSIX_SIGNAL)
#include "prt_signal.h"
#endif
#if defined(OS_OPTION_LINUX)
#include <linux/kthread.h>
#endif
#if defined(OS_OPTION_LOCALE)
#include <locale.h>
#endif
#endif

#define OS_CORES_32 32
#if (OS_MAX_CORE_NUM > OS_CORES_32)
#define OS_CORE_MASK U64
#else
#define OS_CORE_MASK U32
#endif

extern U32 g_tskBaseId;
#if !defined(OS_OPTION_SMP) // AMP
struct TagOsRunQue {
    U32 taskReadyListBitMap;
    /* 优先级bit位表 */
    U32 tskChildBitMap[OS_GET_WORD_NUM_BY_PRIONUM(OS_TSK_NUM_OF_PRIORITIES)];
    struct TagListObject readyList[OS_TSK_NUM_OF_PRIORITIES];
};

struct TagOsTskSortedDelayList {
    /* 延时任务链表 */
    struct TagListObject tskList;
};
#else
struct TagOsTskSortedDelayList {
    /* 超时链涉及核间操作，必须有锁*/
    volatile uintptr_t spinLock;

    U32 reserved;
    /* 延时任务链表 */
    struct TagListObject tskList;
    /* 延时链上最近超时的tick刻度 */
    U64 nearestTicks;
};
struct TagTskCb;
struct TagOsRunQue;

/* 调度类 */
struct TagScheduleClass {
    void (*osEnqueueTask)(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags);
    void (*osDequeueTask)(struct TagOsRunQue *runQue, struct TagTskCb *tsk, U32 flags);
    void (*osPutPrevTask)(struct TagOsRunQue *runQue, struct TagTskCb *prevTsk);
    struct TagTskCb *(*osPickNextTask)(struct TagOsRunQue *runQue);
    struct TagTskCb *(*osNextReadyTask)(struct TagOsRunQue *runQue);
};
#endif

/*
 * 任务线程及进程控制块的结构体统一定义。
 */
struct TagTskCb {
    /* 当前任务的SP */
    void *stackPointer;
    /* 任务状态,后续内部全改成U32 */
    U32 taskStatus;
    /* 任务的运行优先级 */
    TskPrior priority;
    /* 任务栈配置标记 */
    U16 stackCfgFlg;
#if defined(OS_OPTION_SMP)
    
    /* 任务正在进行的操作类型 */
    volatile U32 taskOperating;
    /* 用于任务做关键不可打断的操作时不被打断如绑定，挂起，删任务，被删。0可以做这些事，为1不能做*/
    U32 opBusy;
#endif
    /* 任务栈大小 */
    U32 stackSize;
    TskHandle taskPid;

    /* 任务栈顶 */
    uintptr_t topOfStack;

    /* 任务入口函数 */
    TskEntryFunc taskEntry;
    /* 任务Pend的信号量指针 */
    void *taskSem;

    /* 任务的参数 */
    uintptr_t args[4];
#if (defined(OS_OPTION_TASK_INFO))
    /* 存放任务名 */
    char name[OS_TSK_NAME_LEN];
#endif
    /* 信号量链表指针 */
    struct TagListObject pendList;
    /* 任务延时链表指针 */
    struct TagListObject timerList;
    /* 持有互斥信号量链表 */
    struct TagListObject semBList;
    /* 记录条件变量的等待线程 */
    struct TagListObject condNode;
#if defined(OS_OPTION_LINUX)
    /* 等待队列指针 */
    struct TagListObject waitList;
#endif
#if defined(OS_OPTION_EVENT)
    /* 任务事件 */
    U32 event;
    /* 任务事件掩码 */
    U32 eventMask;
#endif
    /* 任务记录的最后一个错误码 */
    U32 lastErr;
#if defined(OS_OPTION_SMP)
    /* reserved */
    U32 smpReserve;
    struct PushablTskList pushAbleList;
    U32 timeCoreID;
    /* 该任务可以执行的核bitmap */
    OS_CORE_MASK coreAllowedMask;
    /* 该任务可以执行的核个数 */
    U32 nrCoresAllowed;
    /* 该任务所处的rq挂接的核号 */
    U32 coreID;
    /* 该任务是否在运行队列上(ready) */
    bool isOnRq;
    /* 该任务使用的调度类 */
    struct TagScheduleClass *scheClass;
#endif
    /* 任务恢复的时间点(单位Tick) */
    U64 expirationTick;
#if defined(OS_OPTION_POSIX)
    /* 当前任务状态 */
    U8 state;
    /* pthread cancel */
    U8 cancelState;
    U8 cancelType;
    U8 cancelPending;
    struct __ptcb *cancelBuf;
    /* exit status */
    void *retval;
    /* count for thread join */
    U16 joinCount;
    /* semaphore for thread join */
    SemHandle joinableSem;
    /* pthread key */
    void *tsd[PTHREAD_KEYS_MAX];
    U32 tsdUsed;
#if defined(OS_OPTION_POSIX_SIGNAL)
    /* 设置的阻塞信号掩码 */
    signalSet sigMask;
    /* 设置的等待信号掩码 */
    signalSet sigWaitMask;
    /* 未决信号掩码 */
    signalSet sigPending;
    /* 信号信息 */
    struct TagListObject sigInfoList;
    /* 信号处理函数 */
    _sa_handler sigVectors[PRT_SIGNAL_MAX];
    /* 保存任务的原SP */
    void *oldStackPointer;
    int holdSignal;
    timer_t itimer;
#endif
#if defined(OS_OPTION_LOCALE)
    locale_t locale;
#endif
#if defined(OS_OPTION_LINUX)
    struct task_struct *kthreadTsk;
#endif
#endif
#if defined(OS_OPTION_NUTTX_VFS)
    struct filelist tskFileList;
    void *stdio_locks;      // 来自musl中的struct pthread
#if defined(CONFIG_FILE_STREAM)
    struct streamlist ta_streamlist;
#endif
#endif
};

/*
 * 任务信息表节点数据结构
 */
struct TagTskMonNode {
    /* 撑死/饿死的时间点(tick) */
    U64 expiredTick;
    /* 撑死/饿死标记 */
    U32 flag;
    /* 撑死/饿死检测标记 */
    U32 ckFlag;
    /* 检测类型 */
    U32 ckStyles;
    /* 保留(对齐) */
    U32 reserved;
};

typedef void (*TskCoresleep)(void);
typedef void (*TaskNameGetFunc)(U32 taskId, char **taskName);
typedef U32 (*TaskNameAddFunc)(U32 taskId, const char *name);

#if defined(OS_OPTION_SMP)
extern volatile uintptr_t g_createTskLock;
extern struct TagScheduleClass g_osRtSingleSchedClass;
extern struct TagOsTskSortedDelayList g_tskSortedDelay[OS_MAX_CORE_NUM];
extern struct TagListObject g_tskPercpuRecycleList[OS_MAX_CORE_NUM];
#define CPU_TSK_DELAY_BASE(cpu)         (&g_tskSortedDelay[(cpu)])
#define OS_TSK_DELAY_LOCKED_DETACH(task)                                                    \
    do {                                                                                    \
        struct TagOsTskSortedDelayList *tmpDlyBase = &g_tskSortedDelay[(task)->timeCoreID];\
        OS_MCMUTEX_LOCK(0, &tmpDlyBase->spinLock);                                          \
        ListDelete(&(task)->timerList);                                                     \
        OS_MCMUTEX_UNLOCK(0, &tmpDlyBase->spinLock);                                        \
    } while (0)
#else
extern TskHandle g_idleTaskId;
extern struct TagOsRunQue g_runQueue;
extern struct TagTskCb *g_runningTask;
extern struct TagTskCb *g_highestTask;
extern struct TagOsTskSortedDelayList g_tskSortedDelay;
#define GET_RUNQ(core)                  (&g_runQueue)
#define OS_TASK_GET_PRI_LIST(prio)      (&g_runQueue.readyList[(prio)])
#endif

extern U16 g_uniTaskLock;
extern U32 g_tskMaxNum;
extern struct TagTskCb *g_tskCbArray;
extern struct TskModInfo g_tskModInfo;
extern struct TagTskMonNode *g_tskMonList;

extern TskEntryFunc g_tskIdleEntry;
extern TaskNameAddFunc g_taskNameAdd;
extern TaskNameGetFunc g_taskNameGet;

extern volatile TskCoresleep g_taskCoreSleep;

#if defined(OS_OPTION_POWEROFF)
typedef void (*PowerOffFuncT)(void);
extern PowerOffFuncT g_sysPowerOffHook;
extern void OsPowerOffSetFlag(void);
extern void OsPowerOffFuncHook(PowerOffFuncT powerOffFunc);
extern void OsCpuPowerOff(void); /* hook之前异常, 需实现该函数 */

typedef void (*SetOfflineFlagFuncT)(void);
extern void SetOfflineFlagDefaultFunc(PowerOffFuncT powerOffFunc);
extern void OsSetOfflineFlagHook(SetOfflineFlagFuncT setOfflineFlagFunc);
extern SetOfflineFlagFuncT g_setOfflineFlagHook;
#endif
#define OS_TSK_STACK_CFG_BY_USER 1
#define OS_TSK_STACK_CFG_BY_SYS  0

#define OS_TSK_PARA_0   0
#define OS_TSK_PARA_1   1
#define OS_TSK_PARA_2   2
#define OS_TSK_PARA_3   3

/* 定义任务的缺省优先级 */
#define OS_TSK_DEFAULT_PRIORITY 20
#define OS_TSK_PRIO_BIT_MAP_POW 5
#define OS_TSK_STACK_TOP_MAGIC  0xAAAAAAAA

#define OS_TSK_PRIO_RDY_BIT  0x80000000U

#define OS_TSK_OP_FREE          0
#define OS_TSK_OP_MIGRATING     (1UL << 0)  // 任务在迁移中，看到delete立即返回
#define OS_TSK_OP_SUSPENDING    (1UL << 2)  // 任务在挂起中，看到delete立即返回
#define OS_TSK_OP_RESUMING      (1UL << 3)  // 任务在恢复中，看到delete立即返回
#define OS_TSK_OP_DELETING      (1UL << 9)  // 任务在删除中，必须等待上述迁移、挂起操作完
#define OS_TSK_OP_MOVING        (1UL << 10) // 任务在迁移/回迁过程中标志，确保不会被他人操作该任务

/* 定义任务的缺省任务栈大小 */
#define TSK_IS_UNUSED(tsk)             ((tsk)->taskStatus == OS_TSK_UNUSED)
#define TSK_STATUS_TST(tsk, statBit)   (((tsk)->taskStatus & (statBit)) != 0)
#define TSK_STATUS_CLEAR(tsk, statBit) ((tsk)->taskStatus &= ~(statBit))
#define TSK_STATUS_SET(tsk, statBit)   ((tsk)->taskStatus |= (statBit))
#define OS_TSK_LOCK()                  (OS_TASK_LOCK_DATA++)
#define GET_TCB_PEND(ptr)              LIST_COMPONENT(ptr, struct TagTskCb, pendList)
// 保留一个idle task。最大任务handle为FE，FF表示硬中断线程。
#define MAX_TASK_NUM                   ((1U << OS_TSK_TCB_INDEX_BITS) - 2)  // 254
#define OS_TSK_BLOCK                   (OS_TSK_DELAY | OS_TSK_PEND | OS_TSK_SUSPEND  | OS_TSK_QUEUE_PEND | \
        OS_TSK_EVENT_PEND | OS_TSK_WAITQUEUE_PEND | OS_TSK_DELETING)

#if defined(OS_OPTION_LINUX)
#define KTHREAD_TSK_STATE_TST(tsk, tskState)   (((tsk)->kthreadTsk != NULL) && ((tsk)->kthreadTsk->state == (tskState)))
#define KTHREAD_TSK_STATE_SET(tsk, tskState)   ((tsk)->kthreadTsk != NULL ? (tsk)->kthreadTsk->state = (tskState) : 0)
#endif

#if defined(OS_OPTION_SMP)
#define TSK_IDX2PID(taskID)             (taskID)    // ((taskID) + g_tskBaseID)
#define TSK_GET_INDEX(taskID)           (taskID)    // ((taskID) - g_tskBaseID)
#define OS_TASK_LOCK_DATA               (THIS_RUNQ()->uniTaskLock)
#define IDLE_TASK_ID                    (THIS_RUNQ()->tskIdlePID)
#define RUNNING_TASK                    (THIS_RUNQ()->tskCurr)
#define TSK_CORE_GET(taskPID)           (GET_TCB_HANDLE(taskPID)->coreID)
#define TSK_CORE_SET(tsk, coreId)       ((tsk)->coreID = (coreId))
#define TSK_CREATE_DEL_LOCK()           OS_MCMUTEX_LOCK(0, &g_createTskLock)
#define TSK_CREATE_DEL_UNLOCK()         OS_MCMUTEX_UNLOCK(0, &g_createTskLock)
#define OS_MAX_TCB_NUM                  (g_tskMaxNum + 1 + g_maxNumOfCores)  // 1个给无效线程，其余的为每个核的预留idle
#define OS_TSK_SUSPEND_READY_BLOCK      (OS_TSK_SUSPEND | OS_TSK_CRG_IDLE_SUSPEND | OS_TSK_DELETING | OS_TSK_READY)
#define OS_TSK_UNLOCK()                 OsTskUnlock()
#define CPUMASK_FIRST_BIT(mask)         OsGetRMB(mask)
#define GET_TASK_RQ(task)               (GET_RUNQ((task)->coreID))
#define OS_TASK_GET_PRI_LIST(prio)      (&((struct TagOsRunQue *)THIS_RUNQ())->rtRq.activeTsk.readyList[(prio)])

#define CHECK_TSK_PID_OVERFLOW(taskID)  (TSK_GET_INDEX(taskID) >= (OS_MAX_TCB_NUM - 1))
#define CHECK_TSK_PID(taskID)           (TSK_GET_INDEX(taskID) >= g_tskMaxNum)

#define CPU_OVERTIME_SORT_LIST_LOCK(tskDlyBase) \
    OS_MCMUTEX_LOCK(0, &(tskDlyBase)->spinLock) // 提高性能专用锁，避免多次cpu偏移计算地址
#define CPU_OVERTIME_SORT_LIST_UNLOCK(tskDlyBase) OS_MCMUTEX_UNLOCK(0, &(tskDlyBase)->spinLock) // 提高性能专用锁

#if(OS_MAX_CORE_NUM > 1)
#define OS_TSK_OP_SET(taskCB, operate)      ((taskCB)->taskOperating |= (operate))
#define OS_TSK_OP_CLR(taskCB, operate)      ((taskCB)->taskOperating &= ~(operate))
#else
#define OS_TSK_OP_SET(taskCB, operate)      ((void)(taskCB), (void)(operate))
#define OS_TSK_OP_CLR(taskCB, operate)      ((void)(taskCB), (void)(operate))
#endif

#else
#define OS_TASK_LOCK_DATA          g_uniTaskLock
#define IDLE_TASK_ID               g_idleTaskId
#define RUNNING_TASK               g_runningTask
#define TSK_GET_INDEX(taskId)      ((taskId) - g_tskBaseId)

/* 内核进程的进程及线程调度控制块使用同一类型 */
#define OS_MAX_TCB_NUM             (g_tskMaxNum + 1 + 1)  // 1个IDLE，1个无效任务

#define OS_TSK_DELAY_LOCKED_DETACH(task)            ListDelete(&(task)->timerList)
#define CHECK_TSK_PID_OVERFLOW(taskId)              (TSK_GET_INDEX(taskId) >= (g_tskMaxNum + 1))
#endif
#define GET_TCB_HANDLE_BY_TCBID(index)  (((struct TagTskCb *)g_tskCbArray) + (uintptr_t)(index))
#define GET_TCB_HANDLE(taskPid)        (GET_TCB_HANDLE_BY_TCBID(TSK_GET_INDEX((uintptr_t)(taskPid))))
#define OS_PST_ZOMBIE_TASK             (GET_TCB_HANDLE_BY_TCBID(OS_MAX_TCB_NUM - 1 ))

#define OS_TSK_SUSPEND_READY_BLOCK (OS_TSK_SUSPEND)
// 设置任务优先级就绪链表主BitMap中Bit位，每32个优先级对应一个BIT位，即Bit0(优先级0~31),Bit1(优先级32~63),依次类推。
#define OS_SET_RDY_TSK_BIT_MAP(priority) \
        (OS_TSK_PRIO_RDY_BIT >> ((priority) >> OS_TSK_PRIO_BIT_MAP_POW))
// 清除任务优先级就绪链表主BitMap中Bit位，每32个优先级对应一个BIT位，即Bit0(优先级0~31),Bit1(优先级32~63),依次类推。
#define OS_CLR_RDY_TSK_BIT_MAP(priority) \
        (~(OS_TSK_PRIO_RDY_BIT >> ((priority) >> OS_TSK_PRIO_BIT_MAP_POW)))

// 设置任务优先级就绪链表子BitMap中Bit位，每个优先级对应一个BIT位。
#define OS_SET_CHILD_BIT_MAP(priority) \
        (OS_TSK_PRIO_RDY_BIT >> ((priority) % OS_WORD_BIT_NUM))
// 清除任务优先级就绪链表子BitMap中Bit位，每个优先级对应一个BIT位。
#define OS_CLR_CHILD_BIT_MAP(priority) \
        (~(OS_TSK_PRIO_RDY_BIT >> ((priority) % OS_WORD_BIT_NUM)))

extern void OsTaskScan(void);
extern void OsTskUnlock(void);
extern void OsTskSchedule(void);
extern void OsTskEntry(TskHandle taskId);
extern void OsTaskExit(struct TagTskCb *tsk);
extern void OsTskReadyAdd(struct TagTskCb *task);
extern void OsTskReadyDel(struct TagTskCb *taskCb);
extern void OsTskSwitchHookCaller(U32 prevPid, U32 nextPid);
extern void OsContextSave(struct TagTskCb *taskCB);
extern void OsTskTimerAdd(struct TagTskCb *taskCb, uintptr_t timeout);

extern U32 OsTskMaxNumGet(void);
extern U32 OsTaskDelete(TskHandle taskPid);
#if defined(OS_OPTION_SMP)
extern U32 OsTaskCreateOnly(TskHandle *taskPid, struct TskInitParam *initParam, bool isSmpIdle);
#else
extern U32 OsTaskCreateOnly(TskHandle *taskPid, struct TskInitParam *initParam);
#endif

#if !defined(OS_OPTION_SMP)
extern void OsTskScheduleFast(void);
extern void OsTskScheduleFastPs(uintptr_t intSave);
#endif
/*
 * 模块内内联函数定义
 */

#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE void OsTskScheduleFast(void)
{
    OsTskSchedule();
}
OS_SEC_ALW_INLINE INLINE void OsTskScheduleFastPs(uintptr_t intSave)
{
    (void)(intSave);
    OsTskSchedule();
}
OS_SEC_ALW_INLINE INLINE bool IsIdleTask(TskHandle task)
{
    return (((task) >= g_tskMaxNum) && ((task) < (g_tskMaxNum + g_maxNumOfCores)));
}

OS_SEC_ALW_INLINE INLINE void OsTskSetCoreAllowed(U32 coreAllowed, struct TagTskCb *tsk)
{
    tsk->coreAllowedMask = coreAllowed;
    tsk->nrCoresAllowed = 1;
}
OS_SEC_ALW_INLINE INLINE void OsSetTaskCoreId(U32 newCoreId, struct TagTskCb *tskCB)
{
    tskCB->coreID = newCoreId;
}
#endif

#if !defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE void OsTskHighestSet(void)
{
    U32 rdyListIdx;
    struct TagListObject *readyList = NULL;
    U32 childBitMapIdx;

    /* find the highest priority */
    /* get valid Child BitMap according to the ReadyListBitMap */
    childBitMapIdx = OsGetLmb1(g_runQueue.taskReadyListBitMap);

    /* get the ready list task priority idx in the Child BitMap */
    rdyListIdx = OsGetLmb1(g_runQueue.tskChildBitMap[childBitMapIdx]);

    /* get task ready list according to the task priority */
    readyList = &(g_runQueue.readyList[OS_GET_32BIT_ARRAY_BASE(childBitMapIdx) + rdyListIdx]);

    g_highestTask = GET_TCB_PEND(OS_LIST_FIRST(readyList));
}
OS_SEC_ALW_INLINE INLINE bool IsIdleTask(TskHandle task)
{
    return (task == IDLE_TASK_ID);
}
#endif
#if defined(OS_OPTION_SMP)
extern void OsTskReadyAddBgd(struct TagTskCb *task);
#else
OS_SEC_ALW_INLINE INLINE void OsTskReadyAddBgd(struct TagTskCb *task)
{
    OsTskReadyAdd(task);
}
#endif
#if (defined(OS_OPTION_SMP) && (OS_MAX_CORE_NUM > 1))
// 必须是多核SMP场景才有根据flag激活任务、唤醒低功耗核的情形
extern void OsTskReadyAddNoWakeUpIpc(struct TagTskCb *task);
#else
OS_SEC_ALW_INLINE INLINE void OsTskReadyAddNoWakeUpIpc(struct TagTskCb *task)
{
    OsTskReadyAddBgd(task);
}
#endif
#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE U32 OsTskGetCoreAllowed(TskHandle taskPID)
{
    return GET_TCB_HANDLE(taskPID)->coreAllowedMask;
} 
#endif

#if defined(OS_OPTION_SMP)
OS_SEC_ALW_INLINE INLINE bool OsTaskStackIsSave(const struct TagTskCb *taskCB)
{
    return !((taskCB->coreID != THIS_CORE()) && (taskCB->stackCfgFlg == OS_TSK_STACK_CFG_BY_USER));
}
#else
#define THIS_RUNQ() ((struct TagOsRunQue *)&g_runQueue)
OS_SEC_ALW_INLINE INLINE void OsRescheduleCurr(struct TagOsRunQue *runQue)
{
    (void)runQue;
}
#endif

#if defined(OS_OPTION_TICK)
OS_SEC_ALW_INLINE INLINE void OsTaskAttachToTimerList(struct TagTskCb *task, U32 timeOut)
{
    if (timeOut == OS_WAIT_FOREVER) {
        return;
    }

    TSK_STATUS_SET(task, OS_TSK_TIMEOUT);
    OsTskTimerAdd(task, timeOut);
}

OS_SEC_ALW_INLINE INLINE void OsTaskDetachFromTimerList(struct TagTskCb *task)
{
    if (!TSK_STATUS_TST(task, OS_TSK_TIMEOUT)) {
        return;
    }

    OS_TSK_DELAY_LOCKED_DETACH(task);
    TSK_STATUS_CLEAR(task, OS_TSK_TIMEOUT);
}
#else
OS_SEC_ALW_INLINE INLINE void OsTaskAttachToTimerList(struct TagTskCb *task, U32 timeOut)
{
    (void)task;
    (void)timeOut;
}
OS_SEC_ALW_INLINE INLINE void OsTaskDetachFromTimerList(struct TagTskCb *task)
{
    (void)task;
}
#endif
#if defined(OS_OPTION_SMP)
extern U32 g_tskMaxNum;
extern volatile uintptr_t g_createTskLock;

#define OS_TSK_ID_BITS_NUM        10
#define OS_TSK_INDEX_MASK         ((1U << OS_TSK_ID_BITS_NUM) -1)

#define OS_TSK_COMPOSE(tId)       (tId)
#endif

#endif /* PRT_TASK_EXTERNAL_H */
