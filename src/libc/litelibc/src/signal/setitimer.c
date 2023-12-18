#include <sys/time.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <time.h>
#include "prt_signal.h"
#include "prt_signal_external.h"

void deliverSigAlm(union sigval sv)
{
    signalInfo info = {0};
    info.si_signo = SIGALRM;
    info.si_code = SI_USER;
    uintptr_t intSave = OsIntLock();
    PRT_SignalDeliver(sv.sival_int, &info);
    OsIntRestore(intSave);
}

int setitimer(int which, const struct itimerval *restrict new, struct itimerval *restrict old)
{
    int ret = 0;
    struct itimerspec setTime = {{0, 0}, {0, 0}};
    struct itimerspec oldTime = {{0, 0}, {0, 0}};
    struct TagTskCb *taskCb = RUNNING_TASK;
    struct sigevent evp = { 0 };
    evp.sigev_notify = SIGEV_THREAD;
    evp.sigev_notify_function = deliverSigAlm;
    evp.sigev_value.sival_int = taskCb->taskPid;
    if (which != ITIMER_REAL || new == NULL) {
        errno = EINVAL;
        return -1;
    }
    uintptr_t intSave = OsIntLock();
    if (!taskCb->itimer) {
        ret = timer_create(CLOCK_REALTIME, &evp, &taskCb->itimer);
        OsIntRestore(intSave);
    }

    TIMEVAL_TO_TIMESPEC(&new->it_value, &setTime.it_value);
    TIMEVAL_TO_TIMESPEC(&new->it_value, &setTime.it_value);

    ret = timer_settime(taskCb->itimer, 0, &setTime, &oldTime);
    if (ret == 0 && old != NULL) {
        TIMESPEC_TO_TIMEVAL(&old->it_value, &oldTime.it_value);
        TIMESPEC_TO_TIMEVAL(&old->it_interval, &oldTime.it_interval);
    }

    OsIntRestore(intSave);
    return ret;
}
