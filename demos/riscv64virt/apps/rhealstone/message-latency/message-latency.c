/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include <prt_task.h>
#include <prt_sem.h>
#include <prt_sys.h>
#include <prt_queue.h>
#include <prt_clk.h>
#include <riscv.h>
#include "../support/banchmark_support.h"

#define MESSAGE_SIZE (sizeof(long) * 4)
#define BENCHMARKS      100000      
#define WARM_UP_TIMES   500

uintptr_t telapsed;
uintptr_t tloopOverhead;
uintptr_t treceiveOverhead;
U32 count;
TskHandle taskIds[2];
U32 queueId;
long messageBuffer[4];

void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    size_t size;
    for(int i=0; i<WARM_UP_TIMES; i++) {
        (void)PRT_QueueWrite(queueId, messageBuffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);
        (void)PRT_QueueRead(queueId, messageBuffer, (U32*)&size, OS_WAIT_FOREVER);
    }

    /* Put a message in the queue so recieve overhead can be found. */
    (void)PRT_QueueWrite(queueId, messageBuffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);
    /* status up second task, get preempted */
    PRT_TaskResume(taskIds[1]);
    //PRT_Printf("PRT_TaskResume %d\n",status);

    for ( ; count < BENCHMARKS; count++) {
        (void)PRT_QueueWrite(queueId, messageBuffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);
    }

    /* should never reach here */
    rtems_test_assert(false);
}

void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    size_t size = MESSAGE_SIZE;
    /* find recieve overhead - no preempt or task switch */
    benchmark_timer_initialize();
    size = MESSAGE_SIZE;
    (void)PRT_QueueRead(queueId, messageBuffer, (U32*)&size, OS_WAIT_FOREVER);
    treceiveOverhead = benchmark_timer_read();
    /* Benchmark code */
    benchmark_timer_initialize();
    for (int count = 0; count < BENCHMARKS - 1; count++) {
        size = MESSAGE_SIZE;
        (void)PRT_QueueRead(queueId, messageBuffer, (U32*)&size, OS_WAIT_FOREVER);
    }
    telapsed = benchmark_timer_read();
    PRT_Printf("tsk used total cycle    : %ld cyc\n",telapsed);
    PRT_Printf("tsk loop cycle total    : %ld cyc\n",tloopOverhead);
    PRT_Printf("treceiveOverhead cycle  : %ld cyc\n",treceiveOverhead);
    
    U64 cycle = trans(telapsed,BENCHMARKS - 1,tloopOverhead,treceiveOverhead);
    PRT_Printf("tsk msg-latency cycle   : %ld cyc\n",cycle);
    PRT_Printf("tsk msg-latency time    : %ld us\n",PRT_ClkCycle2Us(cycle));
    PRT_SysReboot();
}

void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam taskParam = {0};
    U32 status;
    status = PRT_QueueCreate(1, MESSAGE_SIZE, &queueId);
    PRT_Printf("message_queue_create %d\n",status);

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[0], &taskParam);
    PRT_Printf("PRT_TaskCreate of TA01 %d\n",status);

    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[1], &taskParam);
    PRT_Printf("PRT_TaskCreate of TA02 %d\n",status);

    benchmark_timer_initialize();
    for (int i = 0; i < BENCHMARKS - 1; i++) {
        __asm__ __volatile__("");
    }
    tloopOverhead = benchmark_timer_read();

    status = PRT_TaskResume(taskIds[0]);
    PRT_Printf("PRT_TaskResume of TA01 %d\n",status);

    TskHandle selfTaskId;
    PRT_TaskSelf(&selfTaskId);
    status = PRT_TaskDelete(selfTaskId);
    PRT_Printf("PRT_TaskDelete of SELF %d\n",status);
}
