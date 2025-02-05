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
 * Description: dw_uart8250 驱动
 */
#include "uart.h"
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define UART_PRT_BUF_SIZE 512
#define READ_REG(reg) (*((volatile unsigned char *)(UART0 + reg)))
#define WRITE_REG(reg, value) ((*(((volatile unsigned char *)(UART0 + reg)))) = (value))

void uart_init()
{
  WRITE_REG(LCR, LCR_DLAB);
  U32 divisor = CLK_HZ / (BAUD_RATE << 4);
  WRITE_REG(DLL, (U8)(divisor & 0xff));
  WRITE_REG(DLH, (U8)((divisor >> 8) & 0xff));
  U8 lcr_bit = LCR_CS8 | LCR_1_STB | LCR_PDIS;
  WRITE_REG(LCR, lcr_bit);

  WRITE_REG(FCR, (FCR_FIFO | FCR_FIFO_8 | FCR_RCVRCLR | FCR_XMITCLR));
  WRITE_REG(IER, 1);
}

int uart_getc()
{
  if (READ_REG(LSR) & LSR_RXRDY)
  {
    return READ_REG(RBR);
  }
  else
  {
    return -1;
  }
}

void uart_putc(const char ch)
{
  while (!(READ_REG(LSR) & LSR_TEMT))
    ;
  WRITE_REG(THR, ch);
}

U32 uart_printf(char *fmt, ...)
{
  va_list vlist;
  va_start(vlist, fmt);
  static char uart_prt_buf[UART_PRT_BUF_SIZE];
  int size = vsprintf(uart_prt_buf, fmt, vlist);
  if (size < UART_PRT_BUF_SIZE)
  {
    for (int i = 0; i < size; i++)
    {
      uart_putc(uart_prt_buf[i]);
    }
  }
  return size;
}