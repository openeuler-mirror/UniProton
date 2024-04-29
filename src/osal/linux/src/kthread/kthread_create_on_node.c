#include <linux/err.h>
#include <linux/kthread.h>
#include <stdarg.h>
#include <stdio.h>

#if defined(OS_OPTION_SMP)
#include "../../../core/kernel/task/smp/prt_task_internal.h"
#else
#include "../../../core/kernel/task/amp/prt_task_internal.h"
#endif
#include "prt_mem.h"
#include "prt_posix_internal.h"
#include "prt_sem.h"
#include "prt_task.h"

static void OsKthreadWrapper(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param3;
    (void)param4;
    int (*threadfn)(void *data) = (void *)param1;

    int ret = threadfn((void *)param2);
    PRT_PthreadExit((void *)ret);
}

/**
 * kthread_create_on_node - create a kthread.
 * @threadfn: the function to run until signal_pending(current).
 * @data: data ptr for @threadfn.
 * @node: task and thread structures for the thread are allocated on this node
 * @namefmt: printf-style name for the thread.
 *
 * Description: This helper function creates and names a kernel
 * thread.  The thread will be stopped: use wake_up_process() to start
 * it.  See also kthread_run().  The new thread has SCHED_NORMAL policy and
 * is affine to all CPUs.
 *
 * If thread is going to be bound on a particular cpu, give its node
 * in @node, to get NUMA affinity for kthread stack, or else give NUMA_NO_NODE.
 * When woken, the thread will run @threadfn() with @data as its
 * argument. @threadfn() can either call do_exit() directly if it is a
 * standalone thread for which no one will call kthread_stop(), or
 * return when 'kthread_should_stop()' is true (which means
 * kthread_stop() has been called).  The return value should be zero
 * or a negative error number; it will be passed to kthread_stop().
 *
 * Returns a task_struct or ERR_PTR(-ENOMEM) or ERR_PTR(-EINTR).
 */

// TASK_UNINTERRUPTIBLE 状态在调度时要相应的改变
// 参考 PRT_TaskCreate， __pthread_create
struct task_struct *kthread_create_on_node(int (*threadfn)(void *data),
    void *data, int node, const char namefmt[], ...)
{
    U32 ret;
    U32 taskId;
    uintptr_t intSave;
    uintptr_t *topStack = NULL;
    void *stackPtr = NULL;
    struct TagTskCb *taskCb = NULL;
    uintptr_t curStackSize = 0;

    char *kthread_name = PRT_MemAlloc((U32)OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, sizeof(TASK_COMM_LEN));
    if (kthread_name == NULL) {
        return ERR_PTR(-ENOMEM);
    }

    struct TskInitParam param = {0};
    struct task_struct *taskStrut = NULL;

    va_list ap;
    va_start(ap, namefmt);
    vsnprintf(kthread_name, TASK_COMM_LEN, namefmt, ap);
    va_end(ap);

    param.taskEntry = (TskEntryFunc)OsKthreadWrapper;
    param.taskPrio = 25;
    param.name = kthread_name;
    param.stackSize = g_tskModInfo.defaultSize;
    param.args[OS_TSK_PARA_0] = (uintptr_t)threadfn;
    param.args[OS_TSK_PARA_1] = (uintptr_t)data;
    param.args[OS_TSK_PARA_2] = 0;
    param.args[OS_TSK_PARA_3] = 0;

    ret = OsTaskCreateParaCheck(&taskId, &param);
    if (ret != OS_OK) {
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)kthread_name));
        return ERR_PTR(-EINTR);
    }

    taskStrut = PRT_MemAlloc((U32)OS_MID_APP, OS_MEM_DEFAULT_FSC_PT, sizeof(struct task_struct));
    if (taskStrut == NULL) {
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)kthread_name));
        return ERR_PTR(-ENOMEM);
    }

    intSave = OsIntLock();
#if defined(OS_OPTION_SMP)
    ret = OsTaskCreateChkAndGetTcb(&taskCb, FALSE);
#else
    ret = OsTaskCreateChkAndGetTcb(&taskCb);  
#endif
    if (ret != OS_OK) {
        OsIntRestore(intSave);
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)kthread_name));
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)taskStrut));
        return ERR_PTR(-ENOMEM);
    }

    taskId = taskCb->taskPid;
    taskCb->stackCfgFlg = OS_TSK_STACK_CFG_BY_SYS;
    taskCb->kthreadTsk = taskStrut;
    taskStrut->state = TASK_UNINTERRUPTIBLE;
    taskStrut->pid = taskId;
    taskStrut->name = kthread_name;
    taskStrut->flags = 0;

    // 创建 task，之后如果失败，挂到g_tskCbFreeList上释放
    ret = OsTaskCreateRsrcInit(taskId, &param, taskCb, &topStack, &curStackSize);
    if (ret != OS_OK) {
        ListAdd(&taskCb->pendList, &g_tskCbFreeList);
        OsIntRestore(intSave);
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)kthread_name));
        OS_ERR_RECORD(PRT_MemFree((U32)OS_MID_APP, (void *)taskStrut));
        return ERR_PTR(-EINTR);
    }
    OsTskStackInit(curStackSize, (uintptr_t)topStack);

    stackPtr = OsTskContextInit(taskId, curStackSize, topStack, (uintptr_t)OsTskEntry);

    OsTskCreateTcbInit((uintptr_t)stackPtr, &param, (uintptr_t)topStack, curStackSize, taskCb);

    taskCb->taskStatus = OS_TSK_SUSPEND | OS_TSK_INUSE;

    if (taskCb->state == PTHREAD_CREATE_JOINABLE) {
        ret = PRT_SemCreate(0, &taskCb->joinableSem);
        if (ret != OS_OK) {
            OsIntRestore(intSave);
            OsTaskDelete(taskId);
            return ERR_PTR(-EINTR);
        }
    }

    OsIntRestore(intSave);
    return taskStrut;
}