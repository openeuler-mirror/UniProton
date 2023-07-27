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

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

TskHandle task_ids[2];
const char *task_name[2];

uint32_t telapsed;
uint32_t tloop;
uint32_t tswitch;

unsigned long count1;
U32 status;

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{

    status = PRT_TaskResume(task_ids[1]);
    directive_failed(status, "PRT_TaskResume of TA02");

    tswitch = benchmark_timer_read();

    benchmark_timer_initialize();

    for(count1 = 0; count1 < BENCHMARKS; count1++) {
        PRT_TaskResume(task_ids[1]);
    }


    rtems_test_assert(false);
}

void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{

    benchmark_timer_initialize();
    PRT_TaskSuspend(task_ids[1]);


    for(; count1 < BENCHMARKS - 1;) {
        PRT_TaskSuspend(task_ids[1]);
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
    struct TskInitParam param;

    param.taskEntry = (TskEntryFunc)Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_08;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&task_ids[0], &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    param.taskEntry = (TskEntryFunc)Task02;
    param.stackSize = 0x0800;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&task_ids[1], &param);
    directive_failed(status, "PRT_TaskCreate of TA02");


    benchmark_timer_initialize();
    for (count1 = 0; count1 < (BENCHMARKS * 2) - 1; count1++); {
        /* PRT_TaskResume(task_ids[1]) */
        asm volatile("");
    }
    tloop = benchmark_timer_read();

    status = PRT_TaskResume(task_ids[0]);
    directive_failed(status, "PRT_TaskResume of TA01");

    TskHandle taskId;
    PRT_TaskSelf(&taskId);
    status = PRT_TaskDelete(taskId);
    directive_failed(status, "PRT_TaskDelete of INIT");
}