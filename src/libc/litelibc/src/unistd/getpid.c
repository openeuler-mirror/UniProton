#include <unistd.h>
#include "prt_task.h"

pid_t getpid(void)
{
    TskHandle taskPid = 0;
    PRT_TaskSelf(&taskPid);
    return (pid_t)taskPid;
}