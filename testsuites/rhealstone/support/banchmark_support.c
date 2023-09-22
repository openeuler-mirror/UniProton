/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2020. All rights reserved
 */
#include <stdint.h>
#include <prt_tick.h>

U32 g_cycle = 0;
U64 g_tick = 0;
#define OS_SYSTICK_RELOAD_REG 0xE000E014
#define OS_SYSTICK_CURRENT_REG 0xE000E018

void benchmark_timer_initialize(void)
{
    g_tick = PRT_TickGetCount();
    g_cycle = *(volatile U32 *)OS_SYSTICK_CURRENT_REG;
}

uint32_t benchmark_timer_read(void)
{
    U32 end = *(volatile U32 *)OS_SYSTICK_CURRENT_REG;
    U32 tick = PRT_TickGetCount() - g_tick;
    U32 reload = *(volatile U32 *)OS_SYSTICK_RELOAD_REG;

    if (g_cycle >= end) {
        return (g_cycle - end) + tick * reload;
    } else {
        return end - g_cycle + tick * reload;
    }
}