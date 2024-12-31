#include "hal_uart_dw.h"
#include "uart.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#define PRINT_BUFFER_SIZE 128
int uart_printf(const char *fmt, ...)
{
        va_list args;
        char buf[PRINT_BUFFER_SIZE];
        int count;
        int pos;
        static int message_id = 1;
        int msg_id_now;
        msg_id_now = message_id++;
        pos = snprintf(buf, sizeof(buf) - 1, "[UniProton %d] :", msg_id_now);

        va_start(args, fmt);
        vsnprintf(buf + pos, sizeof(buf) - 1 - pos, fmt, args);
        va_end(args);

        /* Use putchar directly as 'puts()' adds a newline. */
        buf[PRINT_BUFFER_SIZE - 1] = '\0';
        count = uart_put_buff(buf);
        return (count + pos);
}

void uart_init(void)
{
        int baudrate = 115200;
        int uart_clock = 25 * 1000 * 1000;

        /* set uart to pinmux_uart1 */
        hal_uart_init(UART0, baudrate, uart_clock);
        uart_puts("UniProton : uart init done");
}

uint8_t uart_putc(uint8_t ch)
{
        if (ch == '\n') {
                hal_uart_putc('\r');
        }
        hal_uart_putc(ch);
        return ch;
}

void uart_puts(const char *str)
{
        if (!str)
                return;

        while (*str) {
                uart_putc(*str++);
        }
}

int uart_getc(void)
{
        return (int)hal_uart_getc();
}

int uart_tstc(void)
{
        return hal_uart_tstc();
}


int uart_put_buff(char *buf)
{
        int flags;
        int count = 0;

        while (buf[count]) {
                if (uart_putc(buf[count]) != '\n') {
                        count++;
                } else {
                        break;
            }
        }
        return count;
}

