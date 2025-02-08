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
#define WARM_UP_TIMES 1000
static uintptr_t telapsed;
static uintptr_t tloopOverhead;
static uintptr_t treceiveOverhead;
static U32 count;
static TskHandle taskIds[2];
static U32 queueId;
static long messageBuffer[4];

static void Task01(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 status;
    
    /* Put a message in the queue so recieve overhead can be found. */
    (void)PRT_QueueWrite(queueId, messageBuffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);

    /* status up second task, get preempted */
    status = PRT_TaskResume(taskIds[1]);
    directive_failed(status, "PRT_TaskResume");

    for (; count < BENCHMARKS; count++) {
        (void)PRT_QueueWrite(queueId, messageBuffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);
    }
}

static void Task02(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    size_t size = MESSAGE_SIZE;

    /* find recieve overhead - no preempt or task switch */
#if defined(OS_ARCH_ARMV7_M)
    benchmark_timer_initialize();
    size = MESSAGE_SIZE;
    (void)PRT_QueueRead(queueId, messageBuffer, (U32 *)&size, OS_WAIT_FOREVER);
    treceiveOverhead = benchmark_timer_read();
#else
    treceiveOverhead = 0;
    for(int i =0; i < WARM_UP_TIMES-1; i++) {
    	benchmark_timer_initialize();
    	size = MESSAGE_SIZE;
    	(void)PRT_QueueRead(queueId, messageBuffer, (U32 *)&size, OS_WAIT_FOREVER);
    	treceiveOverhead += benchmark_timer_read();
        (void)PRT_QueueWrite(queueId, messageBuffer, MESSAGE_SIZE, OS_WAIT_FOREVER, OS_QUEUE_NORMAL);
    }
    benchmark_timer_initialize();
    size = MESSAGE_SIZE;
    (void)PRT_QueueRead(queueId, messageBuffer, (U32 *)&size, OS_WAIT_FOREVER);
    treceiveOverhead += benchmark_timer_read();
    treceiveOverhead /= WARM_UP_TIMES;
#endif

    /* Benchmark code */
    benchmark_timer_initialize();
    for (count = 0; count < BENCHMARKS - 1; count++) {
        size = MESSAGE_SIZE;
        (void)PRT_QueueRead(queueId, messageBuffer, (U32 *)&size, OS_WAIT_FOREVER);
    }
    telapsed = benchmark_timer_read();

    put_time(
        "Rhealstone: Intertask Message Latency",
        telapsed,
        BENCHMARKS - 1,
        tloopOverhead,
        treceiveOverhead
    );
}

void MessageLatencyTest()
{
    struct TskInitParam taskParam = {0};
    U32 status;

    status = PRT_QueueCreate(1, MESSAGE_SIZE, &queueId);
    directive_failed(status, "message_queue_create");

    taskParam.taskEntry = Task01;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA01";
    taskParam.taskPrio = OS_TSK_PRIORITY_20;
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

    benchmark_timer_initialize();
    for (count = 0; count < BENCHMARKS - 1; count++) {
        __asm__ __volatile__("");
    }
    tloopOverhead = benchmark_timer_read();

    status = PRT_TaskResume(taskIds[0]);
    directive_failed(status, "PRT_TaskResume of TA01");
}

#if !defined(LOSCFG_SHELL_TEST)
void Init(uintptr_t paraml, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    MessageLatencyTest();
}
#endif
