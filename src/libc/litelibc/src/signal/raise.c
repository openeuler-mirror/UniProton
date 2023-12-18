#include <signal.h>
#include <stdint.h>
#include "pthread_impl.h"
#include "prt_signal.h"
#include "prt_signal_external.h"

int raise(int sig)
{
    return kill(RUNNING_TASK->taskPid, sig);
}
