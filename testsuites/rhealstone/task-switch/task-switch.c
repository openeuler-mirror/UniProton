/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include <prt_task.h>
#include <prt_sys.h>
#include "tmacros.h"
#include "timesys.h"
#include "prt_hwi.h"

#define BENCHMARKS 50000

TskHandle taskIds[2];
uintptr_t loopCycle;
uintptr_t dirOverhead;
U32 count1, count2;
U32 status;

void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    uintptr_t telapsed;

    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    
    benchmark_timer_initialize();

    for (count1 = 0; count1 < BENCHMARKS - 1; count1++) {
        PRT_TaskDelay(0);
    }

    telapsed = benchmark_timer_read();
    put_time(
        "Rhealstone: Task switch", telapsed, (BENCHMARKS * 2) - 1,
        loopCycle,
        dirOverhead);

    PRT_SysReboot();
}

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    status = PRT_TaskResume(taskIds[1]);
    directive_failed(status, "PRT_TaskResume of TA02");

    PRT_TaskDelay(0);

    for (count2 = 0; count2 < BENCHMARKS; count2++) {
        PRT_TaskDelay(0);
    }

    rtems_test_assert(false);
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam param = { 0 };

    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;

    param.taskEntry = (TskEntryFunc)Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&taskIds[0], &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    param.taskEntry = (TskEntryFunc)Task02;
    param.stackSize = 0x800;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&taskIds[1], &param);
    directive_failed(status, "PRT_TaskCreate of TA02");

    benchmark_timer_initialize();
    for (count1 = 0; count1 < BENCHMARKS - 1; count1++) {
        __asm__ __volatile__("");
    }
    for (count2 = 0; count2 < BENCHMARKS; count2++) {
        __asm__ __volatile__("");
    }
    loopCycle = benchmark_timer_read();

#if defined(OS_ARCH_ARMV7_M)
    PRT_TaskDelay(0);
    benchmark_timer_initialize();
    PRT_TaskDelay(0);
    dirOverhead = benchmark_timer_read();
#else
    dirOverhead = 0;
    for(int i = 0; i < BENCHMARKS; i++) {
    	benchmark_timer_initialize();
    	PRT_TaskDelay(0);
    	dirOverhead += benchmark_timer_read();
    }
    dirOverhead /= BENCHMARKS;
#endif

    status = PRT_TaskResume(taskIds[0]);
    directive_failed(status, "PRT_TaskResume of TA01");

    TskHandle taskId;
    PRT_TaskSelf(&taskId);
    status = PRT_TaskDelete(taskId);
    directive_failed(status, "PRT_TaskDelete of INIT");
}
