#include <linux/current.h>
#include "prt_task_external.h"

struct task_struct *get_current(void) 
{
    return RUNNING_TASK->kthreadTsk;
}