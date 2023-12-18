#include <linux/wait.h>
#include "prt_sem.h"
#include "prt_task_external.h"

// 参考 OsTaskScan
void wake_up(struct wait_queue_head *wq_head)
{
    struct TagTskCb *taskCb = NULL;
    uintptr_t intSave;

    intSave = OsIntLock();
    LIST_FOR_EACH(taskCb, &wq_head->waitList, struct TagTskCb, waitList) {
        ListDelete(&taskCb->waitList);
        OsTskReadyAdd(taskCb);
        KTHREAD_TSK_STATE_SET(taskCb, TASK_RUNNING);
        TSK_STATUS_CLEAR(taskCb, OS_TSK_WAITQUEUE_PEND);
        taskCb = LIST_COMPONENT(&wq_head->waitList, struct TagTskCb, waitList);
    }
    OsTskSchedule();
    OsIntRestore(intSave);
}

// 参考 PRT_TaskDelay 以及 OsSemPendListPut
OS_SEC_L0_TEXT U32 enter_wait_queue(struct wait_queue_head *wq_head)
{
    uintptr_t intSave;
    struct TagTskCb *runTask = NULL;

    intSave = OsIntLock();
    if (UNI_FLAG == 0) {
        OsIntRestore(intSave);
        // 初始化过程中不允许切换
        return OS_ERRNO_TSK_DELAY_IN_INT;
    }

    if (OS_INT_ACTIVE) {
        OS_REPORT_ERROR(OS_ERRNO_TSK_DELAY_IN_INT);
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_DELAY_IN_INT;
    }

    if (OS_TASK_LOCK_DATA != 0) {
        OsIntRestore(intSave);
        return OS_ERRNO_TSK_DELAY_IN_LOCK;
    }
    runTask = RUNNING_TASK;
    OsTskReadyDel(runTask);
    TSK_STATUS_SET(runTask, OS_TSK_WAITQUEUE_PEND);
    KTHREAD_TSK_STATE_SET(runTask, TASK_UNINTERRUPTIBLE);
    ListTailAdd(&runTask->waitList, &wq_head->waitList);

    OsTskSchedule();
    OsIntRestore(intSave);
    return OS_OK;
}