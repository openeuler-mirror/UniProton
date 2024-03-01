/*
 * Copyright (c) 2014 Daniel Ramirez. (javamonn@gmail.com)
 *
 * This file's license is 2-clause BSD as in this distribution's LICENSE file.
 */

#include <prt_task.h>
#include <prt_sys.h>
#include <prt_clk.h>
#include <riscv.h>
#include "../support/banchmark_support.h"

#define BENCHMARKS 50000

TskHandle taskIds[2];
uintptr_t loopCycle;
uintptr_t dirOverhead;
U32 count1, count2;
U32 status;

void Task02(uintptr_t param1, uintptr_t param2, uintptr_t param3,uintptr_t param4)
{
    w_mstatus(((r_mstatus()&(~MIE_S))));
    uintptr_t telapsed;
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    benchmark_timer_initialize();
    for (count1 = 0; count1 < BENCHMARKS; count1++) {
        PRT_TaskDelay(0);
    }
    telapsed = benchmark_timer_read();
    PRT_Printf("tsk used total cycle    : %ld cyc\n",telapsed);
    PRT_Printf("tsk loop cycle total    : %ld cyc\n",loopCycle);
    U64 cycle = trans(telapsed,(BENCHMARKS * 2),(loopCycle+dirOverhead),0);
    PRT_Printf("single switch with lock cycle : %ld cyc\n",dirOverhead/(BENCHMARKS*2));
    PRT_Printf("tsk single switch cycle : %ld cyc\n",cycle);
    PRT_Printf("tsk single switch time  : %ld us\n",PRT_ClkCycle2Us(cycle));
    PRT_SysReboot();
}

void Task01(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    w_mstatus(((r_mstatus()&(~MIE_S))));
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    status = PRT_TaskResume(taskIds[1]);
    if(status != OS_OK) {
        PRT_Printf("error in prt_task resume \n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    PRT_TaskDelay(0);
    for (count2 = 0; count2 < BENCHMARKS; ) {
        PRT_TaskDelay(0);
    }
    rtems_test_assert(false);
}

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    struct TskInitParam param = { 0 };
    (void)param1;
    (void)param2;
    (void)param3;
    (void)param4;
    w_mstatus(((r_mstatus()&(~MIE_S))));
    param.taskEntry = (TskEntryFunc)Task01;
    param.stackSize = 4096;
    param.name = "TA01";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;

    status = PRT_TaskCreate(&taskIds[0], &param);
    if(status != OS_OK) {
        PRT_Printf("error in prt_task_create \n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    param.taskEntry = (TskEntryFunc)Task02;
    param.stackSize = 4096;
    param.name = "TA02";
    param.taskPrio = OS_TSK_PRIORITY_05;
    param.stackAddr = 0;
    status = PRT_TaskCreate(&taskIds[1], &param);
    if(status != OS_OK) {
        PRT_Printf("error in prt_task_create \n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }
    benchmark_timer_initialize();
    for (count1 = 0; count1 < BENCHMARKS; count1++) {
        __asm__ __volatile__("");
    }
    for (count2 = 0; count2 < BENCHMARKS; count2++) {
        __asm__ __volatile__("");
    }
    loopCycle = benchmark_timer_read();

    PRT_TaskDelay(0);
    for(int i=0;i<BENCHMARKS*2;i++)
    {
        benchmark_timer_initialize();
        PRT_TaskDelay(0);
        dirOverhead += benchmark_timer_read();
    }

    status = PRT_TaskResume(taskIds[0]);
    if(status != OS_OK) {
        PRT_Printf("error in prt_task resume \n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    TskHandle taskId;
    PRT_TaskSelf(&taskId);
    status = PRT_TaskDelete(taskId);
    PRT_Printf("PRT_TaskDelete of INIT ERROR\n");
}
