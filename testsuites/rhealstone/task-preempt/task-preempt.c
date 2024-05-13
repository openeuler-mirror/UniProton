/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include <prt_task.h>
#include <prt_sys.h>
#include "tmacros.h"
#include "timesys.h"

#define BENCHMARKS 50000
#define WARMUP_TIMES 1000

TskHandle taskIds[2];
uintptr_t telapsed;
uintptr_t tloop;
uintptr_t tswitch;
U32 count, count1;
U32 status;

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
#if defined(OS_ARCH_ARMV7_M)
    for (count = 0; count < WARMUP_TIMES; count++) {
        status = PRT_TaskResume(taskIds[1]);
        tswitch = benchmark_timer_read();
        directive_failed(status, "PRT_TaskResume of TA02");
    }
#else
    for (count = 0; count < WARMUP_TIMES; count++) {
        status = PRT_TaskResume(taskIds[1]);
        tswitch += benchmark_timer_read();
        directive_failed(status, "PRT_TaskResume of TA02");
    }
    tswitch /= WARMUP_TIMES;
#endif

    benchmark_timer_initialize();

    for (count = 0; count < BENCHMARKS; count++) {
        PRT_TaskResume(taskIds[1]);
    }

    rtems_test_assert(false);
}

void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    for (count1 = 0; count1 < WARMUP_TIMES; count1++) {
        benchmark_timer_initialize();
        PRT_TaskSuspend(taskIds[1]);
    }

    for (; count < BENCHMARKS - 1;) {
        PRT_TaskSuspend(taskIds[1]);
    }

    telapsed = benchmark_timer_read();
    put_time(
        "Rhealstone: Task Preempt",
        telapsed,
        BENCHMARKS - 1,
        tloop,
        tswitch
    );

    PRT_SysReboot();
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam param = { 0 };

    param.taskEntry = (TskEntryFunc)Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_08;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&taskIds[0], &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    param.taskEntry = (TskEntryFunc)Task02;
    param.stackSize = 0x0800;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&taskIds[1], &param);
    directive_failed(status, "PRT_TaskCreate of TA02");

    benchmark_timer_initialize();
    for (count = 0; count < (BENCHMARKS * 2) - 1; count++) {
        __asm__ __volatile__("");
    }
    tloop = benchmark_timer_read();

    status = PRT_TaskResume(taskIds[0]);
    directive_failed(status, "PRT_TaskResume of TA01");

    TskHandle taskId;
    PRT_TaskSelf(&taskId);
    status = PRT_TaskDelete(taskId);
    directive_failed(status, "PRT_TaskDelete of INIT");
}
