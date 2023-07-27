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
#include <prt_queue.h>

#define MESSAGE_SIZE (sizeof(long) * 4)
#define BENCHMARKS 50000

void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);
void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4);

uint32_t Telapsed;
uint32_t Tloop_overhead;
uint32_t Treceive_overhead;
uint32_t Count;
TskHandle Task_id[2];
U32 Queue_id;
long Buffer[4];

void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam taskParam = { 0 };
    U32 status;
    
    status = PRT_QueueCreate(1, MESSAGE_SIZE, &Queue_id);
    directive_failed(status, "rtems_message_queue_create");

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&Task_id[0], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA01");

    taskParam.taskEntry = Task02;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA02";
    taskParam.taskPrio = OS_TSK_PRIORITY_18;
    taskParam.stackAddr = 0;
    status = PRT_TaskCreate(&Task_id[1], &taskParam);
    directive_failed(status, "PRT_TaskCreate of TA02");

    benchmark_timer_initialize();
    for (Count = 0; Count < BENCHMARKS - 1; Count++) {
        /* message send/recieve */
        asm volatile("");
    }
    Tloop_overhead = benchmark_timer_read();

    status = PRT_TaskResume(Task_id[0]);
    directive_failed(status, "PRT_TaskResume of TA01");

    TskHandle selfTaskId;
    PRT_TaskSelf(&selfTaskId);
    status = PRT_TaskDelete(selfTaskId);
    directive_failed(status, "PRT_TaskDelete of RTEMS_SELF");  
}

void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 status;

    /* Put a message in the queue so recieve overhead can be found. */
    (void)PRT_QueueWrite(Queue_id, Buffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);

    /* status up second task, get preempted */
    status = PRT_TaskResume(Task_id[1]);
    directive_failed(status, "PRT_TaskResume");

    for (; Count < BENCHMARKS; Count++) {
        (void)PRT_QueueWrite(Queue_id, Buffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);
    }

    /* should never reach here */
    rtems_test_assert(false);
}

void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    size_t size = MESSAGE_SIZE;
    U32 status;

    /* find recieve overhead - no preempt or task switch */
    benchmark_timer_initialize();
    size = MESSAGE_SIZE;
    (void)PRT_QueueRead(Queue_id, Buffer, &size, OS_WAIT_FOREVER);
    Treceive_overhead = benchmark_timer_read();

    /* Benchmark code */
    benchmark_timer_initialize();
    for (Count = 0; Count < BENCHMARKS - 1; Count++) {
        size = MESSAGE_SIZE;
        (void)PRT_QueueRead(Queue_id, Buffer, &size, OS_WAIT_FOREVER);
    }
    Telapsed = benchmark_timer_read();

   put_time(
        "Rhealstone: Intertask Message Latency",
        Telapsed,
        BENCHMARKS - 1,                              /*  */
        Tloop_overhead,
        Treceive_overhead
   );

   PRT_SysReboot();
}