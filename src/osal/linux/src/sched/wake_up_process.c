#include <linux/sched.h>
#include "prt_task_external.h"

/**
 * wake_up_process - Wake up a specific process
 * @p: The process to be woken up.
 *
 * Attempt to wake up the nominated process and move it to the set of runnable
 * processes.
 *
 * Return: 1 if the process was woken up, 0 if it was already running.
 *
 * This function executes a full memory barrier before accessing the task state.
 */

// 如果用了prt_delay 或是 prt_sempend等函数，无法被PRT_TaskResume唤醒
// schedule_timeout, 等待，需要移除等待队列
// PRT_TaskResume OsMoveTaskToReady 修改，OsMoveTaskToReady
// 参考 linux wake_up_process
int wake_up_process(struct task_struct *tsk)
{
    uintptr_t intSave = OsIntLock();
    U32 ret;

    if (!(tsk->state & TASK_NORMAL)) {
        OsIntRestore(intSave);
        return 0;
    }

    // PRT_TaskResume之后会立即调度，所以要改内部的
    ret = PRT_TaskResume(tsk->pid);
    OsIntRestore(intSave);
    if (ret == OS_OK) {
        return 1;
    }
    return 0;
}