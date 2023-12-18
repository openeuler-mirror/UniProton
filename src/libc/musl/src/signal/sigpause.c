#include <signal.h>

int sigpause(int sig)
{
    sigset_t mask;
    sigprocmask(0, 0, &mask);
    int ret = sigdelset(&mask, sig);
    if (ret != 0) {
        return ret;
    }
    return sigsuspend(&mask);
}
