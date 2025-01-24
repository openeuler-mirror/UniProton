#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

#define RHEALSTONE_CMD_NUMS 6

const char *g_rhealstoneCmdStr[RHEALSTONE_CMD_NUMS] = {
    "deadlock-break", "interrupt-latency", "message-latency",
    "semaphore-shuffle", "task-preempt", "task-switch"
};

typedef void (*test_rhealstone)();

extern void DeadLockBreakTest();
extern void InterruptLatencyTest();
extern void MessageLatencyTest();
extern void SemaphoreShuffleTest();
extern void TaskPreemptTest();
extern void TaskSwitchTest();

test_rhealstone g_rhealstoneCmdFunc[RHEALSTONE_CMD_NUMS] = {
    DeadLockBreakTest, InterruptLatencyTest, MessageLatencyTest,
    SemaphoreShuffleTest, TaskPreemptTest, TaskSwitchTest
};

int OsShellCmdRhealstone(int argc, const char **argv)
{
    if (argc == 1) {
        for (int i = 0; i < RHEALSTONE_CMD_NUMS; i++) {
            if (!strcmp(g_rhealstoneCmdStr[i], argv[0])) {
                g_rhealstoneCmdFunc[i]();
                return OS_OK;
            }
        }
    }

    printf("\nUsage: rhealstone [testcase]\n");
    return OS_OK;
}
