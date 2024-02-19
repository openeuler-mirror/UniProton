#include <unistd.h>
#include <errno.h>
#include <sys/resource.h>
#include <limits.h>

int nice(int inc)
{
    int prio;
    int ret;

    errno = 0;
    ret = getpriority(PRIO_PROCESS, 0);
    if (errno != 0) {
        return ret;
    }

    prio = ret + inc;
    ret = setpriority(PRIO_PROCESS, 0, prio);
    if (ret < 0) {
        return ret;
    }

    return prio;
}
