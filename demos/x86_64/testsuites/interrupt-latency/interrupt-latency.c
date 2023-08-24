/* Copyright 2014 Daniel Ramirez (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE.2 file.
 */

/*
 *  WARNING!!!!!!!!!
 *
 *  THIS TEST USES INTERNAL RTEMS VARIABLES!!!
 */

#include "tmacros.h"
#include "timesys.h"
#include <prt_task.h>
#include <prt_sys.h>
#include <prt_hwi.h>

#define BENCHMARKS 50000
#define INTERRUPT_LATENCY_TEST_INT 0xfd

U64 timerOverhead;
U64 interruptTime;

void Isr_handler(U32 intNum)
{
    /* See how long it took system to recognize interrupt */
    interruptTime = benchmark_timer_read();
    PRT_HwiClearPendingBit(intNum);
}

void Task_1(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 ret;
    U32 prio = 0;
    uintptr_t intSave;

    ret = PRT_HwiSetAttr(INTERRUPT_LATENCY_TEST_INT, prio, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        printf("PRT_HwiSetAttr error: %x\n", ret);
        return;
    }
    ret = PRT_HwiCreate(INTERRUPT_LATENCY_TEST_INT, (HwiProcFunc)Isr_handler, (U32)INTERRUPT_LATENCY_TEST_INT);
    if (ret != OS_OK) {
        printf("PRT_HwiCreate error: %x\n", ret);
        return;
    }

    PRT_HwiEnable(INTERRUPT_LATENCY_TEST_INT);
    intSave = PRT_HwiLock();
    OsTrigerHwi(INTERRUPT_LATENCY_TEST_INT);
    /* Benchmark code */
    benchmark_timer_initialize();
    /* goes to Isr_handler */
    PRT_HwiRestore(intSave);
    PRT_HwiDisable(INTERRUPT_LATENCY_TEST_INT);
    put_time(
        "Rhealstone: Interrupt Latency",
        interruptTime,
        1,                              /* Only Rhealstone that isn't an anverage */
        timerOverhead,
        0
    );

    PRT_SysReboot();
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    U32 status;
    TskHandle taskId;
    struct TskInitParam taskParam = { 0 };

    taskParam.taskEntry = (TskEntryFunc)Task_1;
    taskParam.stackSize = 0x800;
    taskParam.name = "TA1";
    taskParam.taskPrio = OS_TSK_PRIORITY_10;
    taskParam.stackAddr = 0;

    status = PRT_TaskCreate(&taskId, &taskParam);
    directive_failed(status, "PRT_TaskCreate of Task_1");

    status = PRT_TaskResume(taskId);
    directive_failed(status, "PRT_TaskResume of Task_1");

    benchmark_timer_initialize();
    timerOverhead = benchmark_timer_read();

    TskHandle selfTaskId;
    PRT_TaskSelf(&selfTaskId);
    status = PRT_TaskDelete(selfTaskId);
    directive_failed(status, "PRT_TaskDelete of SELF");
}
