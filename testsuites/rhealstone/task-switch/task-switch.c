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
#define OS_SYSTICK_RELOAD_REG 0xE000E014
#define OS_SYSTICK_CURRENT_REG 0xE000E018

void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

TskHandle task_ids[2];
const char *task_name[2];
uint32_t loop;
uint32_t dir_overhead;
unsigned long count1, count2;
unsigned long count11 = 0;
unsigned long count22 = 0;
U32 status;

void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    param1 = param1;
    param2 = param2;
    param3 = param3;
    param4 = param4;
    uint32_t telapsed;


    benchmark_timer_initialize();

    for (count1 = 0; count1 < BENCHMARKS - 1; count1++) {
        PRT_TaskDelay(0);
    }

    telapsed = benchmark_timer_read();
    put_time(
        "Rhealstone: Task switch", telapsed, (BENCHMARKS * 2) - 1,
        loop,
        dir_overhead
    );

    PRT_SysReboot();
}

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    param1 = param1;
    param2 = param2;
    param3 = param3;
    param4 = param4;
    status = PRT_TaskResume(task_ids[1]);
    directive_failed(status, "PRT_TaskResume of TA02");

    PRT_TaskDelay(0);

    for (count2 = 0; count2 < BENCHMARKS; count2++) {
        PRT_TaskDelay(0);
    }

    rtems_test_assert(false);
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    param1 = param1;
    param2 = param2;
    param3 = param3;
    param4 = param4;
    struct TskInitParam param;

    param.taskEntry = (TskEntryFunc)Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&task_ids[0], &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    param.taskEntry = (TskEntryFunc)Task02;
    param.stackSize = 0x800;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&task_ids[1], &param);
    directive_failed(status, "PRT_TaskCreate of TA02");


    benchmark_timer_initialize();
    for (count1 = 0; count1 < BENCHMARKS - 1; count1++) {
        /* PRT_TaskDelay(0) */
        asm volatile("");
    }
    for (count2 = 0; count2 < BENCHMARKS; count2++) {
        /* PRT_TaskDelay(0) */
        asm volatile("");
    }
    loop = benchmark_timer_read();

    benchmark_timer_initialize();
    PRT_TaskDelay(0);
    dir_overhead = benchmark_timer_read();

    status = PRT_TaskResume(task_ids[0]);
    directive_failed(status, "PRT_TaskResume of TA01");

    TskHandle taskId;
    PRT_TaskSelf(&taskId);
    status = PRT_TaskDelete(taskId);
    directive_failed(status, "PRT_TaskDelete of INIT");
}