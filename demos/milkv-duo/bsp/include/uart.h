#ifndef _UART_H_
#define _UART_H_

#include <stdint.h>
void uart_init(void);
int uart_getc(void);
int uart_tstc(void);
uint8_t uart_putc(uint8_t ch);
void uart_puts(const char *str);
int uart_put_buff(char *buf);
int uart_printf(const char *fmt, ...);


#endif      
