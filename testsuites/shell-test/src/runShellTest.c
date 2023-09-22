#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "rtt_viewer.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"
#include "prt_sem.h"
#include "runShellTest.h"

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    int runCount = 0;
    int failCount = 0;
    int i;
    int ret = 0;
    test_run_main *run;

    printf("Start ipc testing....\n");

    for (i = 0; i < sizeof(run_shell_test_arry_1)/sizeof(test_run_main *); i++) {
        run = run_shell_test_arry_1[i];
        printf("Runing %s test...\n", run_shell_test_name_1[i]);
        ret = run();
        if (ret != 0) {
            failCount++;
            printf("Run %s test fail\n", run_shell_test_name_1[i]);
        }
    }
    runCount += i;

    printf("Run total testcase %d, failed %d\n", runCount, failCount);
}