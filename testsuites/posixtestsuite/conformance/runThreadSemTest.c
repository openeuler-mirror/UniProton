#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "prt_buildef.h"
#include "securec.h"
#ifdef OS_ARCH_ARMV7_M
#include "rtt_viewer.h"
#endif
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "runThreadSemTest.h"

#define _XOPEN_SOURCE 600
#include <unistd.h>

long sysconf(int name)
{
    switch(name) {
        case _SC_CPUTIME:
        case _SC_THREAD_CPUTIME:
        case _SC_MONOTONIC_CLOCK:
            return 1;
        case _SC_SEM_NSEMS_MAX:
            return OS_SEM_COUNT_MAX;
        default:
            return 0;
    }
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    int runCount = 0;
    int failCount = 0;
    int i;
    int ret = 0;
    test_run_main *run;

    printf("Start sem testing....\n");

    for (i = 0; i < sizeof(run_test_arry_1)/sizeof(test_run_main *); i++) {
        run = run_test_arry_1[i];
        printf("Runing %s test...\n", run_test_name_1[i]);
        ret = run();
        if (ret != 0) {
            failCount++;
            printf("Run %s test fail\n", run_test_name_1[i]);
        }
    }
    runCount += i;

    printf("Run total testcase %d, failed %d\n", runCount, failCount);
}