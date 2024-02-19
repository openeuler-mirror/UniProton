#include <sys/resource.h>
#include <errno.h>
#include <pthread.h>
#include <unistd.h>

int getpriority(int which, id_t who)
{
    struct sched_param param;
    int ret, policy;

    if (which > PRIO_USER || which < PRIO_PROCESS) {
        errno = ENOTSUP;
        return -1;
    }

    if (who == 0) {
        who = getpid();
    }

    ret = pthread_getschedparam((pthread_t)who, &policy, &param);
    if (ret < 0) {
        return ret;
    }

    /* Since -1 is a legal return value, clear errno to avoid the chaos */

    errno = 0;

    return param.sched_priority;
}
