/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-02-22
 * Description: rhealstone support cde
 */
#ifndef BANCHMARK_SUPPORT_H
#define BANCHMARK_SUPPORT_H

#include <uart.h>
#include <platform.h>

extern uintptr_t g_cycle;
#define PRT_Printf uart_printf
#define rtems_test_assert(x) 
#define trans( _total_time, _iterations, _loop_overhead, _overhead) (((_total_time) - (_loop_overhead)) / (_iterations)) - (_overhead)
#define benchmark_timer_initialize()  g_cycle = (*(U64 *)(CLINT_TIME))
#define benchmark_timer_read()       ((*(U64 *)(CLINT_TIME)) - g_cycle)


#endif