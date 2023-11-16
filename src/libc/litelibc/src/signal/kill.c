#include <errno.h>
#include <signal.h>
#include "prt_signal.h"

int kill(pid_t pid, int sig)
{
    TskHandle taskId = (TskHandle)pid;
    signalInfo info = {0};
    info.si_signo = sig;
    info.si_code = SI_USER;
    U32 ret = PRT_SignalDeliver(taskId, &info);
    switch (ret) {
        case OS_OK:
            return 0;
        case OS_ERRNO_SIGNAL_TSK_NOT_CREATED:
            errno = ESRCH;
            break;
        case OS_ERRNO_SIGNAL_NUM_INVALID:
        default:
            errno = EINVAL;
        break;
    }

    return -1;
}
