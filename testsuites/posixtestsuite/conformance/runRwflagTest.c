#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include "prt_buildef.h"
#include "securec.h"
#ifdef OS_ARCH_ARMV7_M
#include "rtt_viewer.h"
#endif
#include "runRwflagTest.h"
#define _XOPEN_SOURCE 600
#include <unistd.h>


void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    int runCount = 0;
    int failCount = 0;
    int i;
    test_run_main *run;

    printf("Start rwflag testing....\n");

    for (i = 0; i < sizeof(run_test_arry_1) / sizeof(test_run_main *); i++) {
        run = run_test_arry_1[i];
        printf("Runing %s test...\n", run_test_name_1[i]);
        int ret = run();
        if (ret != 0) {
            failCount++;
            printf("Run %s test fail\n", run_test_name_1[i]);
        }
    }
    runCount += i;

    printf("Run total testcase %d, failed %d\n", runCount, failCount);
}