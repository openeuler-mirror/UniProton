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

#if defined(OS_ARCH_ARMV7_M)
#define INTERRUPT_LATENCY_TEST_INT 30
/* ** 中断设置悬起寄存器(SETPEND) 0xE000_E200 -0xE000_E21C */
#define OS_NVIC_SETPEND_BASE 0xE000E200UL
#define NVIC_SET_IRQ_PEND(hwiNum)                                                                                           \
    do {                                                                                                            \
        *(volatile U32 *)((uintptr_t)OS_NVIC_SETPEND_BASE + (((hwiNum) >> 5) << 2)) = 1UL << ((hwiNum) & 0x1FUL);   \
    } while (0)
#endif

#if defined(OS_ARCH_X86_64)
#define INTERRUPT_LATENCY_TEST_INT 0xfd
#endif

#if defined(OS_ARCH_ARMV8)
#define INTERRUPT_LATENCY_TEST_INT 15
#endif

uintptr_t timerOverhead;
uintptr_t interruptTime;

void Isr_handler(U32 intNum)
{
    /* See how long it took system to recognize interrupt */
    interruptTime = benchmark_timer_read();
#if !defined(OS_ARCH_ARMV8)
    PRT_HwiClearPendingBit(intNum);
#endif
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
#if defined(OS_ARCH_X86_64)
    intSave = PRT_HwiLock();
    OsTrigerHwi(INTERRUPT_LATENCY_TEST_INT);
    /* Benchmark code */
    benchmark_timer_initialize();
    /* goes to Isr_handler */
    PRT_HwiRestore(intSave);
#endif

#if defined(OS_ARCH_ARMV7_M)
    /* Benchmark code */
    benchmark_timer_initialize();
    /* goes to Isr_handler */
    NVIC_SET_IRQ_PEND(INTERRUPT_LATENCY_TEST_INT);
#endif

#if defined(OS_ARCH_ARMV8)
    intSave = PRT_HwiLock();
    OsHwiMcTrigger(0xf, INTERRUPT_LATENCY_TEST_INT);
    /* Benchmark code */
    benchmark_timer_initialize();
    /* goes to Isr_handler */
    PRT_HwiRestore(intSave);
    __asm__ __volatile__("dsb sy":::"memory", "cc");
#endif
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
#if defined(OS_ARCH_ARMV7_M)
    benchmark_timer_read();
    benchmark_timer_initialize();
#endif
    timerOverhead = benchmark_timer_read();

    TskHandle selfTaskId;
    PRT_TaskSelf(&selfTaskId);
    status = PRT_TaskDelete(selfTaskId);
    directive_failed(status, "PRT_TaskDelete of SELF");
}
