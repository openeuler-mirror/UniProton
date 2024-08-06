/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "bm_common.h"
#include "bm_uart.h"
#include "hi309x_memmap.h"
#include "sample_common.h"

#define MAIN_PRINT_UART 2

static void sample_init_uart(unsigned int uart_id)
{
    serial_cfg uart_cfg_libck = {
        .hw_uart_no = uart_id,
        .uart_src_clk = UART_CLK,
        .data_bits = UART_DATA_8BIT, /* default data_bits is 8 */
        .stop = UART_STOP_1BIT,
        .pen = UART_VERIFY_DISABLE,
        .baud_rate = 115200 /* default baud_rate is 115200 */
    };
    bm_uart_init(&uart_cfg_libck);
    bm_uart_enable_print(uart_id);
}

void sample_prepare(void)
{
    for (int i = 0; i < CPU_RELEASE_ADDR_LEN; i++) {
        *(volatile unsigned int *)((uintptr_t)(CPU_RELEASE_ADDR + i * 4)) = (unsigned int)(0); // 4 is reg num
    }
    sample_init_uart(MAIN_PRINT_UART);
}
