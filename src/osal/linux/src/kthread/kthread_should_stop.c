#include <linux/kthread.h>
#include <linux/err.h>
#include "prt_task_external.h"

bool kthread_should_stop(void)
{
    struct TagTskCb *tskCb = RUNNING_TASK;
    if (tskCb == NULL) {
        return false;
    }
    return ((tskCb->kthreadTsk->flags & KTHREAD_SHOULD_STOP) != 0);
}