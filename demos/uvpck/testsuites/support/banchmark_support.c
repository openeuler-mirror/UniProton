/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved
 */
#include <stdint.h>
#include <prt_tick.h>

U64 g_cycle = 0;

static __inline __attribute__((always_inline)) U64 __tsc(void)
{
    U32 low, high;

    __asm__ __volatile__ ("mfence");
    __asm__ __volatile__ ("rdtsc" : "=a" (low), "=d" (high));
    return (U64)high << 32 | low;
}

void benchmark_timer_initialize(void)
{
    g_cycle = __tsc();
}

U64 benchmark_timer_read(void)
{
    U64 end = __tsc();

    return end - g_cycle;
}