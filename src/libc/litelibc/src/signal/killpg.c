#include <signal.h>
#include <errno.h>

int killpg(pid_t pgid, int sig)
{
    errno = ENOTSUP;
    return -1;
}
