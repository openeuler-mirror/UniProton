#include "prt_posix_internal.h"
#include "prt_task_external.h"
#include <linux/err.h>
#include <linux/kthread.h>

int kthread_stop(struct task_struct *k)
{
    struct TagTskCb *taskCb = NULL;
    uintptr_t intSave;
    TskHandle taskPid;
    void *retVal;
    int ret;

    if (k == NULL) {
        return -EINTR;
    }

    taskPid = k->pid;
    if (CHECK_TSK_PID_OVERFLOW(taskPid)) {
        return -EINTR;
    }

    intSave = OsIntLock();
    taskCb = (struct TagTskCb *)GET_TCB_HANDLE(taskPid);
    if (TSK_IS_UNUSED(taskCb)) {
        OsIntRestore(intSave);
        ret = PRT_PthreadTimedJoin(taskPid, (void **)&retVal, OS_WAIT_FOREVER);
        if (ret != OS_OK) {
            return -EINTR;
        }
        return (int)retVal;
    }

    // bit 位写入
    k->flags |= KTHREAD_SHOULD_STOP;
    OsIntRestore(intSave);

    // 唤醒线程
    wake_up_process(k);

    // 等待线程结束
    ret = PRT_PthreadTimedJoin(taskPid, (void **)&retVal, OS_WAIT_FOREVER);
    if (ret != OS_OK) {
        return -EINTR;
    }
    return (int)retVal;
}