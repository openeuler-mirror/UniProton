#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
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

#ifdef _XOPEN_SOURCE
#undef _XOPEN_SOURCE
#endif

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

uid_t getuid(void) {
    return 0;
}

extern int rpc_test_entry();
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)(param1);
    (void)(param2);
    (void)(param3);
    (void)(param4);

    rpc_test_entry();
}
