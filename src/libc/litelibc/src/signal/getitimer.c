#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <stddef.h>
#include <time.h>
#include "prt_signal.h"
#include "prt_signal_external.h"

int getitimer(int which, struct itimerval *old)
{
    int ret = 0;
    struct itimerspec getTime = {{0, 0}, {0, 0}};
    struct TagTskCb *taskCb = RUNNING_TASK;
    if (which != ITIMER_REAL || old == NULL) {
        errno = EINVAL;
        return -1;
    }

    if (!taskCb->itimer) {
        return 0;
    }

    ret = timer_gettime(taskCb->itimer, &getTime);
    if (ret == 0) {
        TIMESPEC_TO_TIMEVAL(&old->it_value, &getTime.it_value);
        TIMESPEC_TO_TIMEVAL(&old->it_interval, &getTime.it_interval);
    }

    return ret;
}
