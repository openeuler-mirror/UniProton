#include <nuttx/config.h>

#include <nuttx/wqueue.h>
#include "prt_task.h"

int work_queue(int qid, FAR struct work_s *work, worker_t worker,
               FAR void *arg, clock_t delay)
{
    (void)qid;
    (void)work;

    if (delay != 0) {
        PRT_TaskDelay(delay);
    }

    worker(arg);

    return 0;
}