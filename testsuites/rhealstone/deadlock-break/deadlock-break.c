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

void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Task03(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);

TskHandle Task_id[3];
SemHandle Sem_id;
U32 Status;

uint32_t Count;
uint32_t Telapsed;
uint32_t Tswitch_overhead;
uint32_t Tobtain_overhead;
uint32_t Sem_exe;

void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam taskParam;
    U32 Status;
    TskHandle selfTaskPid;

    PRT_SemCreate(1, &Sem_id);
    directive_failed(Status, "PRT_SemCreate of S0");

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_16;
    taskParam.stackAddr = 0;
    Status = PRT_TaskCreate(&Task_id[0], &taskParam);
    directive_failed(Status, "PRT_TaskCreate of TA01");

    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    Status = PRT_TaskCreate(&Task_id[1], &taskParam);
    directive_failed(Status, "PRT_TaskCreate of TA02");

    taskParam.taskEntry = Task03;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA03";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    Status = PRT_TaskCreate(&Task_id[2], &taskParam);
    directive_failed(Status, "PRT_TaskCreate of TA03");

    /* find overhead of obtaining semaphore*/
    benchmark_timer_initialize();
    PRT_SemPend(Sem_id, OS_WAIT_FOREVER);
    Tobtain_overhead = benchmark_timer_read();
    PRT_SemPost(Sem_id);

    Status = PRT_TaskSelf(&selfTaskPid);
    Status = PRT_TaskSetPriority(selfTaskPid, OS_TSK_PRIORITY_25);

    /* Get time of benchmark with no semaphores involved, i.e. find overhead */
    Sem_exe = 0;
    Status = PRT_TaskResume(Task_id[2]);
    directive_failed(Status, "PRT_TaskResume of TA03");

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_16;
    taskParam.stackAddr = 0;
    Status = PRT_TaskCreate(&Task_id[0], &taskParam);
    directive_failed(Status, "PRT_TaskCreate of TA01");

    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    Status = PRT_TaskCreate(&Task_id[1], &taskParam);
    directive_failed(Status, "PRT_TaskCreate of TA02");

    taskParam.taskEntry = Task03;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA03";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    Status = PRT_TaskCreate(&Task_id[2], &taskParam);
    directive_failed(Status, "PRT_TaskCreate of TA03");
    /* Get time of benchmark with semaphores */
    Sem_exe = 1;
    Status = PRT_TaskResume(Task_id[2]);
    directive_failed(Status, "PRT_TaskResume of TA03");

    /* Should never reach here*/
    rtems_test_assert(false);
}

void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    /* All tasks have bad time to start up once TA01 is running */

    /* Benchmark code */
    benchmark_timer_initialize();
    for (Count = 0; Count < BENCHMARKS; Count++) {
        if (Sem_exe == 1) {
            /* Block on call */
            PRT_SemPend(Sem_id, OS_WAIT_FOREVER);
        }

        if (Sem_exe == 1) {
            /* Release semaphore immediately after obtaining it */
            PRT_SemPost(Sem_id);
        }

        /* Suspend self, go to TA02 */
        PRT_TaskSuspend(Task_id[0]);
    }
    Telapsed = benchmark_timer_read();

    /* Check which run this was */
    if (Sem_exe == 0) {
        Tswitch_overhead = Telapsed;
        PRT_TaskSuspend(Task_id[1]);
        PRT_TaskSuspend(Task_id[2]);
        PRT_TaskSuspend(Task_id[0]);
    } else {
        put_time (
            "Rhealstone: Deadlock Break",
            Telapsed,
            BENCHMARKS,         /* Total number of times deadlock broken */
            Tswitch_overhead,   /* Overhead of loop and task switches */
            Tobtain_overhead
        );
        PRT_SysReboot();
    }
}

void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    /* Start up TA01, get preempted */
    if (Sem_exe == 1) {
        Status = PRT_TaskResume(Task_id[0]);
        directive_failed(Status, "PRT_TaskResume of TA01");
    } else {
        Status = PRT_TaskResume(Task_id[0]);
        directive_failed(Status, "PRT_TaskResume of TA01");
    }

    /* Benchmark code */
    for (; Count < BENCHMARKS;) {
        /* Suspend self, go to TA01 */
        PRT_TaskSuspend(Task_id[1]);

        /* Wake up TA01, get preempted */
        PRT_TaskResume(Task_id[0]);
    }
}

void Task03(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    if (Sem_exe == 1) {
        /* Low priority task holds mutex */
        PRT_SemPend(Sem_id, OS_WAIT_FOREVER);
    }

    /* Start up TA02, get preempted */
    if (Sem_exe == 1) {
        Status = PRT_TaskResume(Task_id[1]);
        directive_failed(Status, "PRT_TaskResume of TA02");
    } else {
        Status = PRT_TaskResume(Task_id[1]);
        directive_failed(Status, "PRT_TaskResume of TA02");
    }

    /* Benchmark code */
    for (; Count < BENCHMARKS;) {
            if (Sem_exe == 1) {
                /* Preempted by TA01 upon release */
                PRT_SemPost(Sem_id);
            }

            if (Sem_exe == 1) {
                /* Prepare for next Benchmark */
                PRT_SemPend(Sem_id, OS_WAIT_FOREVER);
            }
            /* Wake up TA02, get preempted */
            PRT_TaskResume(Task_id[1]);
    }
}
