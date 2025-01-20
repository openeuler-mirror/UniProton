/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2025-01-20
 * Description: uart驱动头文件
 */
#ifndef __UART_H
#define __UART_H
// 硬件bsp板载驱动
#include "prt_typedef.h"
#include "platform.h"

#define THR 0x00  /* Transmitter holding reg. */
#define RBR 0x00  /* Receiver data reg.       */
#define DLL 0x00 /* Baud rate divisor (LSB)  */
#define DLH 0x04 /* Baud rate divisor (MSB)  */
#define IER 0x04  /* Interrupt enable reg.    */
#define IIR 0x08  /* Interrupt ID reg.        */
#define FCR 0x08  /* FIFO control reg.        */
#define LCR 0x0C  /* Line control reg.        */
#define MCR 0x010  /* Modem control reg.       */
#define LSR 0x014  /* Line status reg.         */
#define MSR 0x06  /* Modem status reg.        */

#define LCR_DLAB 0x80   /* divisor latch access enable */
#define LCR_CS8 0x03    /* 8 bits data size */
#define LCR_1_STB 0x00  /* 1 stop bit */
#define LCR_PDIS 0x00   /* parity disable */

#define FCR_FIFO 0x01    /* enable XMIT and RCVR FIFO */
#define FCR_RCVRCLR 0x02 /* clear RCVR FIFO */
#define FCR_XMITCLR 0x04 /* clear XMIT FIFO */
#define FCR_MODE1 0x08 /* set receiver in mode 1 */
#define FCR_FIFO_8 0x80  /* 8 bytes in RCVR FIFO */

#define LSR_RXRDY 0x01 /* receiver data available */
#define LSR_TEMT 0x40  /* transmitter empty */

#define CLK_HZ 24000000
#define BAUD_RATE 115200

void uart_init();
int uart_getc();
void uart_putc(const char ch);
U32 uart_printf(char *fmt, ...);
#endif
