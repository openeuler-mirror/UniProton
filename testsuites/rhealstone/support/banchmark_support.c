/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved
 */
#include <stdint.h>
#include <prt_tick.h>
#include "prt_buildef.h"
#include <prt_clk.h>
uintptr_t g_cycle = 0;
U64 g_tick = 0;

#if defined(OS_ARCH_ARMV7_M)
#define OS_SYSTICK_RELOAD_REG 0xE000E014
#define OS_SYSTICK_CURRENT_REG 0xE000E018
#endif

#if defined(OS_ARCH_X86_64)
static __inline __attribute__((always_inline)) U64 __tsc(void)
{
    U32 low, high;

    __asm__ __volatile__ ("mfence");
    __asm__ __volatile__ ("rdtsc" : "=a" (low), "=d" (high));
    return (U64)high << 32 | low;
}
#endif

void benchmark_timer_initialize(void)
{
#if defined(OS_ARCH_ARMV7_M)
    g_tick = PRT_TickGetCount();
    g_cycle = *(volatile U32 *)OS_SYSTICK_CURRENT_REG;
#endif

#if defined(OS_ARCH_X86_64)
    g_cycle = __tsc();
#endif

#if defined(OS_ARCH_ARMV8)
    __asm__ __volatile__ ("MRS %0, CNTPCT_EL0" : "=r"(g_cycle) :: "memory", "cc");
#endif

#if defined(OS_ARCH_RISCV64)
    g_cycle = PRT_ClkGetCycleCount64();
#endif

}

uintptr_t benchmark_timer_read(void)
{
#if defined(OS_ARCH_ARMV7_M)
    U32 end = *(volatile U32 *)OS_SYSTICK_CURRENT_REG;
    U32 tick = PRT_TickGetCount() - g_tick;
    U32 reload = *(volatile U32 *)OS_SYSTICK_RELOAD_REG;

    if (g_cycle >= end) {
        return (g_cycle - end) + tick * reload;
    } else {
        return end - g_cycle + tick * reload;
    }
#endif

#if defined(OS_ARCH_X86_64)
    U64 end = __tsc();
    return end - g_cycle;
#endif

#if defined(OS_ARCH_ARMV8)
    U64 end;
    __asm__ __volatile__ ("MRS %0, CNTPCT_EL0" : "=r"(end) :: "memory", "cc");
    return end - g_cycle;
#endif

#if defined(OS_ARCH_RISCV64)
    U64 end = PRT_ClkGetCycleCount64();
    return end - g_cycle;
#endif
}
