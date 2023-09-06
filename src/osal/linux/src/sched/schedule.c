#include <linux/sched.h>
#include "prt_task.h"
#include "prt_task_external.h"

void schedule(void)
{
    int ret = PRT_TaskDelay(0);
    if (ret != 0 && ret != OS_ERRNO_TSK_YIELD_NOT_ENOUGH_TASK) {
        printf("[ERROR] schedule fail, %x\n", ret);
    }
}