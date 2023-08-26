/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include "tmacros.h"
#include "timesys.h"
#include <prt_task.h>
#include <prt_sem.h>
#include <prt_sys.h>

#define BENCHMARKS 20000

TskHandle taskIds[3];
SemHandle semId;
U32 status;
U32 count;
U32 semExe;
U64 telapsed;
U64 switchOverhead;
U64 obtainOverhead;

void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    /* All tasks have bad time to start up once TA01 is running */

    /* Benchmark code */
    benchmark_timer_initialize();
    for (count = 0; count < BENCHMARKS; count++) {
        if (semExe == 1) {
            /* Block on call */
            PRT_SemPend(semId, OS_WAIT_FOREVER);
        }

        if (semExe == 1) {
            /* Release semaphore immediately after obtaining it */
            PRT_SemPost(semId);
        }

        /* Suspend self, go to TA02 */
        PRT_TaskSuspend(taskIds[0]);
    }
    telapsed = benchmark_timer_read();

    /* Check which run this was */
    if (semExe == 0) {
        switchOverhead = telapsed;
        PRT_TaskSuspend(taskIds[1]);
        PRT_TaskSuspend(taskIds[2]);
        PRT_TaskSuspend(taskIds[0]);
    } else {
        put_time (
            "Rhealstone: Deadlock Break",
            telapsed,
            BENCHMARKS,         /* Total number of times deadlock broken */
            switchOverhead,   /* Overhead of loop and task switches */
            obtainOverhead
        );
        PRT_SysReboot();
    }
}

void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    /* Start up TA01, get preempted */
    if (semExe == 1) {
        status = PRT_TaskResume(taskIds[0]);
        directive_failed(status, "PRT_TaskResume of TA01");
    } else {
        status = PRT_TaskResume(taskIds[0]);
        directive_failed(status, "PRT_TaskResume of TA01");
    }

    /* Benchmark code */
    for (; count < BENCHMARKS;) {
        /* Suspend self, go to TA01 */
        PRT_TaskSuspend(taskIds[1]);

        /* Wake up TA01, get preempted */
        PRT_TaskResume(taskIds[0]);
    }
}

void Task03(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    if (semExe == 1) {
        /* Low priority task holds mutex */
        PRT_SemPend(semId, OS_WAIT_FOREVER);
    }

    /* Start up TA02, get preempted */
    if (semExe == 1) {
        status = PRT_TaskResume(taskIds[1]);
        directive_failed(status, "PRT_TaskResume of TA02");
    } else {
        status = PRT_TaskResume(taskIds[1]);
        directive_failed(status, "PRT_TaskResume of TA02");
    }

    /* Benchmark code */
    for (; count < BENCHMARKS;) {
        if (semExe == 1) {
            /* Preempted by TA01 upon release */
            PRT_SemPost(semId);
        }

        if (semExe == 1) {
            /* Prepare for next Benchmark */
            PRT_SemPend(semId, OS_WAIT_FOREVER);
        }
        /* Wake up TA02, get preempted */
        PRT_TaskResume(taskIds[1]);
    }
}

void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam taskParam = { 0 };
    U32 status;
    TskHandle selfTaskPid;

    PRT_SemCreate(1, &semId);
    directive_failed(status, "PRT_SemCreate of S0");

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_16;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[0], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA01");

    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[1], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA02");

    taskParam.taskEntry = Task03;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA03";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[2], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA03");

    /* find overhead of obtaining semaphore*/
    benchmark_timer_initialize();
    PRT_SemPend(semId, OS_WAIT_FOREVER);
    obtainOverhead = benchmark_timer_read();
    PRT_SemPost(semId);

    status = PRT_TaskSelf(&selfTaskPid);
    directive_failed(status, "PRT_TaskSelf");
    status = PRT_TaskSetPriority(selfTaskPid, OS_TSK_PRIORITY_25);
    directive_failed(status, "PRT_TaskSetPriority");

    /* Get time of benchmark with no semaphores involved, i.e. find overhead */
    semExe = 0;
    status = PRT_TaskResume(taskIds[2]);
    directive_failed(status, "PRT_TaskResume of TA03");

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_16;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[0], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA01");

    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[1], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA02");

    taskParam.taskEntry = Task03;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA03";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[2], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA03");
    /* Get time of benchmark with semaphores */
    semExe = 1;
    status = PRT_TaskResume(taskIds[2]);
    directive_failed(status, "PRT_TaskResume of TA03");

    /* Should never reach here*/
    rtems_test_assert(false);
}
