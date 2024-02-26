/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include <prt_task.h>
#include <prt_sem.h>
#include <prt_sys.h>
#include <prt_clk.h>
#include <riscv.h>
#include "../support/banchmark_support.h"

#define BENCHMARKS 20000
#define WARM_UP_TIMES 1000

TskHandle taskIds[3];
SemHandle semId;
U32 status;
U32 count;
U32 semExe;
uintptr_t telapsed;
uintptr_t switchOverhead;
uintptr_t obtainOverhead;

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
        U64 cycle = trans(telapsed,BENCHMARKS,switchOverhead,obtainOverhead);
        PRT_Printf("tsk used total time : %ld cyc\n", telapsed);
        PRT_Printf("tsk swth total time : %ld cyc\n", switchOverhead);
        PRT_Printf("obtainOverhead time : %ld cyc\n", obtainOverhead);
        PRT_Printf("Rhealstone     time : %ld cyc\n", cycle);
        PRT_Printf("Rhealstone     time : %ld us\n", PRT_ClkCycle2Us(cycle));
        PRT_SysReboot();
    }
}

void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    /* Start up TA01, get preempted */
    if (semExe == 1) {
        status = PRT_TaskResume(taskIds[0]);
    } else {
        status = PRT_TaskResume(taskIds[0]);
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
    } else {
        status = PRT_TaskResume(taskIds[1]);
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

    status = PRT_SemCreate(1, &semId);
    PRT_Printf("PRT_SemCreate of S0 %d\n",status);

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_16;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[0], &taskParam);
   
    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[1], &taskParam);
   
    taskParam.taskEntry = Task03;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA03";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[2], &taskParam);
   
    /* find overhead of obtaining semaphore*/
    for(int i=0;i<WARM_UP_TIMES;i++)
    {
        benchmark_timer_initialize();
        PRT_SemPend(semId, OS_WAIT_FOREVER);
        obtainOverhead += benchmark_timer_read();
        PRT_SemPost(semId);
    }
    obtainOverhead /= WARM_UP_TIMES;
    

    status = PRT_TaskSelf(&selfTaskPid);
    status = PRT_TaskSetPriority(selfTaskPid, OS_TSK_PRIORITY_25);
   
    /* Get time of benchmark with no semaphores involved, i.e. find overhead */
    semExe = 0;
    status = PRT_TaskResume(taskIds[2]);
   
    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_16;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[0], &taskParam);
   
    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[1], &taskParam);
   
    taskParam.taskEntry = Task03;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA03";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[2], &taskParam);
    // PRT_Printf("PRT_TaskCreate of TA03 %d\n",status);
    /* Get time of benchmark with semaphores */
    semExe = 1;
    status = PRT_TaskResume(taskIds[2]);

    /* Should never reach here*/
    rtems_test_assert(false);
}
