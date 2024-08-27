#ifndef __UART_H
#define __UART_H



void sys_uart_init(void);
#define uart_init(x) sys_uart_init()


void sys_uart_putc(char c);
#define uart_putc(c) sys_uart_putc(c)


int sys_uart_printf(const char * fmt, ...);
#define uart_printf sys_uart_printf

void sys_uart_putstr(char *s);
#define uart_putstr_sync(s) sys_uart_putstr(s)

int sys_uart_getchar();
#define uartgetc sys_uart_getchar

#endif

