/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include <prt_task.h>
#include <prt_sem.h>
#include <prt_sys.h>
#include "tmacros.h"
#include "timesys.h"

#define BENCHMARKS 50000

static void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
static void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

TskHandle task_ids[2];
SemHandle sem_id;

uint32_t telapsed;
uint32_t tswitch;
uint32_t count;
uint32_t sem_flag;

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam param;
    TskHandle pid;
    U32 status;

    status = PRT_SemCreate(1, &sem_id);
    directive_failed(status, "PRT_SemCreate of S0");

    param.taskEntry = Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_19;
    param.stackAddr = 0;
    status = PRT_TaskCreate(&task_ids[0], &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    param.taskEntry = Task02;
    param.stackSize = 0x800;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_19;
    param.stackAddr = 0;
    status = PRT_TaskCreate(&task_ids[1], &param);
    directive_failed(status, "PRT_TaskCreate of TA02");

    status = PRT_TaskSelf(&pid);
    directive_failed(status, "PRT_TaskSelf of self");

    status = PRT_TaskSetPriority(pid, OS_TSK_PRIORITY_25);
    directive_failed(status, "PRT_TaskSetPriority of self");

    sem_flag = 0;
    status = PRT_TaskResume(task_ids[0]);
    directive_failed(status, "PRT_TaskResume of TA01");

    param.taskEntry = Task01;
    param.stackSize = 0x800;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_19;
    param.stackAddr = 0;
    status = PRT_TaskCreate(&task_ids[0], &param);
    directive_failed(status, "PRT_TaskCreate of TA01");

    param.taskEntry = Task02;
    param.stackSize = 0x800;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_19;
    param.stackAddr = 0;
    status = PRT_TaskCreate(&task_ids[1], &param);
    directive_failed(status, "PRT_TaskCreate of TA02");

    sem_flag = 1;
    status = PRT_TaskResume(task_ids[0]);
    directive_failed(status, "benchmark_task_create of TA01");

    rtems_test_assert(false);
}

static void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 status;

    if (sem_flag == 0) {
        status = PRT_TaskResume(task_ids[1]);
        directive_failed(status, "PRT_TaskResume of TA02");
    } else {
        status = PRT_TaskResume(task_ids[1]);
        directive_failed(status, "PRT_TaskResume of TA02");
    }
    PRT_TaskDelay(0);

    for (; count < BENCHMARKS;) {
        if (sem_flag == 1) {
            PRT_SemPend(sem_id, OS_WAIT_FOREVER);
        }
        PRT_TaskDelay(0);

        if (sem_flag == 1) {
            PRT_SemPost(sem_id);
        }
        PRT_TaskDelay(0);
    }

    rtems_test_assert(false);
}

static void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{

    benchmark_timer_initialize();
    for (count = 0; count < BENCHMARKS; count++) {
        if (sem_flag == 1) {
            PRT_SemPend(sem_id, OS_WAIT_FOREVER);
        }
        PRT_TaskDelay(0);

        if (sem_flag == 1) {
            PRT_SemPost(sem_id);
        }
        PRT_TaskDelay(0);
    }
    telapsed = benchmark_timer_read();

    if (sem_flag == 0) {
        tswitch = telapsed;
        PRT_TaskSuspend(task_ids[0]);
        PRT_TaskSuspend(task_ids[1]);
    } else {
        put_time(
            "Rhealstone: Semaphore Shuffle",
            telapsed,
            (BENCHMARKS * 2),
            tswitch,
            0
        );
        PRT_SysReboot();
    }
}