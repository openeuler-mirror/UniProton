#ifndef PL011_H
#define PL011_H

#include "prt_typedef.h"

#define UART_DR                     0x00
#define UART_FR                     0x18
#define UART_ILPR                   0x20
#define UART_IBRD                   0x24
#define UART_FBRD                   0x28
#define UART_LCR_H                  0x2C
#define UART_CR                     0x30
#define UART_IFLS                   0x34
#define UART_IMSC                   0x38
#define UART_MIS                    0x40
#define UART_ICR                    0x44

#define UART_CR_RX_EN               (0x01 << 9)
#define UART_CR_TX_EN               (0x01 << 8)
#define UART_CR_EN                  (0x01 << 0)
#define UART_LCR_H_FIFO_EN          (0x01 << 4)
#define UART_LCR_H_8_BIT            (0x03 << 5)
#define UART_INT_HALF               (0x02 << 3)
#define UART_RXRIS                  (0x01 << 4)
#define UART_RTRIS                  (0x01 << 6)

#define UART_TXFF                   0x20
#define UART_RXFE                   0x10

#define PL011_CLK_DIV               16U
#define PL011_NUM_8                 8U
#define CONSOLE_UART_BAUDRATE       115200U

#define UART_REG(x) (*(volatile U32 *)(UART_BASE_ADDR + (x)))
#define UART_QUEUE_SIZE             1024

U32 UartGetChar(unsigned char *ch, U32 timeout);
void UartPutChar(unsigned char ch);
U32 PRT_UartInterruptInit(void);
#endif
