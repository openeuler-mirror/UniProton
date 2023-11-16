#include <signal.h>
#include <errno.h>
#include "prt_signal.h"
#include "prt_signal_external.h"

int sigsuspend(const sigset_t *mask)
{
    signalSet prtSet = mask->__bits[0];
    (void)PRT_SigSuspend(&prtSet);
    errno = EINTR;

    return -1;
}
