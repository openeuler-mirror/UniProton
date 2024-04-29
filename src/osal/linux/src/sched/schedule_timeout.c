#include <linux/sched.h>
#include <linux/printk.h>
#include "prt_task_external.h"
#if defined(OS_OPTION_SMP)
#include "../../../core/kernel/task/smp/prt_task_internal.h"
#else
#include "../../../core/kernel/task/amp/prt_task_internal.h"
#endif

/**
 * schedule_timeout - sleep until timeout
 * @timeout: timeout value in jiffies
 *
 * Make the current task sleep until @timeout jiffies have
 * elapsed. The routine will return immediately unless
 * the current task state has been set (see set_current_state()).
 *
 * You can set the task state as follows -
 *
 * %TASK_RUNNING - does not sleep at all
 *
 * %TASK_UNINTERRUPTIBLE - at least @timeout jiffies are guaranteed to
 * pass before the routine returns unless the current task is explicitly
 * woken up, (e.g. by wake_up_process())".
 *
 * %TASK_INTERRUPTIBLE - the routine may return early if a signal is
 * delivered to the current task or the current task is explicitly woken
 * up.
 *
 * The current task state is guaranteed to be TASK_RUNNING when this
 * routine returns.
 *
 * Specifying a @timeout value of %MAX_SCHEDULE_TIMEOUT will schedule
 * the CPU away without a bound on the timeout. In this case the return
 * value will be %MAX_SCHEDULE_TIMEOUT.
 *
 * Returns 0 when the timer has expired otherwise the remaining time in
 * jiffies will be returned.  In all cases the return value is guaranteed
 * to be non-negative.
 */

// 参考 linux schedule_timeout, PRT_TaskDelay,OsSemPendListPut
// OsTaskScan 修改
long schedule_timeout(long timeout)
{
    uintptr_t intSave;
    struct TagTskCb *runTask = NULL;
    U64 expire;

    intSave = OsIntLock();
    runTask = RUNNING_TASK;

    if (UNI_FLAG == 0) {
        goto SCHEDULE_NO_SLEEP;
    }

    if (OS_INT_ACTIVE) {
        OS_REPORT_ERROR(OS_ERRNO_TSK_DELAY_IN_INT);
        goto SCHEDULE_NO_SLEEP;
    }

    if (OS_TASK_LOCK_DATA != 0) {
        goto SCHEDULE_NO_SLEEP;
    }

    if (timeout < 0) {
        printk(KERN_ERR "schedule_timeout: wrong timeout "
                    "value %lx\n", timeout);
        goto SCHEDULE_NO_SLEEP;
    }

    if (KTHREAD_TSK_STATE_TST(runTask, TASK_RUNNING) || timeout == 0) {
        OsIntRestore(intSave);
        schedule();
        return timeout;
    }

    OsTskReadyDel(runTask);
    TSK_STATUS_SET(runTask, OS_TSK_DELAY_INTERRUPTIBLE);
    // 如果是永久等待，则不加 timer
    if (timeout == MAX_SCHEDULE_TIMEOUT) {
        OsTskScheduleFastPs(intSave);
        OsIntRestore(intSave);
        return timeout;
    }

    /* 如果不是永久等待则将任务挂到计时器链表中，设置OS_TSK_TIMEOUT是为了判断是否等待超时 */
    OsTskTimerAdd(runTask, timeout);
    TSK_STATUS_SET(runTask, OS_TSK_TIMEOUT);
    expire = runTask->expirationTick;

    OsTskScheduleFastPs(intSave);
    OsIntRestore(intSave);
    timeout = 0;
    if (expire > g_uniTicks) {
        timeout = (long)(expire - g_uniTicks);
    }
    return timeout < 0 ? 0 : timeout;

SCHEDULE_NO_SLEEP:
    OsIntRestore(intSave);
    KTHREAD_TSK_STATE_SET(runTask, TASK_RUNNING);
    return timeout < 0 ? 0 : timeout;
}