#include "serial.h"
#include "securec.h"
#include "common.h"

serial_t g_sys_serial;
#define HW_UART_NO  (g_sys_serial.cfg.hw_uart_no)
#define UART_CLK    (PERI_APB_FREQ)
#define EAGAIN      11

void serial_soft_init(serial_cfg *cfg, uart_ops *hw_ops)
{
    if (!cfg || !hw_ops) {
        return;
    }

    g_sys_serial.cfg.init_done = 0;
    g_sys_serial.cfg.hw_uart_no = SERIAL_SEL_UART_PORT;
    g_sys_serial.cfg.uart_src_clk = UART_CLK;
    strncpy_s(g_sys_serial.cfg.name, SERIAL_NAME_SIZE, cfg->name, SERIAL_NAME_SIZE - 1);
    g_sys_serial.cfg.data_bits = cfg->data_bits;
    g_sys_serial.cfg.stop = cfg->stop;
    g_sys_serial.cfg.pen = cfg->pen;
    g_sys_serial.cfg.eps = cfg->eps;
    g_sys_serial.cfg.baud_rate = cfg->baud_rate;
    g_sys_serial.hw_ops = hw_ops;

    g_sys_serial.cfg.init_done = 1;

    return;
}

void serial_init(serial_cfg *cfg, uart_ops *hw_ops)
{
    if (!cfg || !hw_ops) {
        return;
    }

    serial_soft_init(cfg, hw_ops);

    g_sys_serial.cfg.init_done = 0;

    if (g_sys_serial.hw_ops->init(&g_sys_serial.cfg)) {
        return;
    }

    g_sys_serial.cfg.init_done = 1;

    return;
}

void serial_putc(const char ch)
{
    uart_ops *hw_ops = g_sys_serial.hw_ops;

    if (g_sys_serial.cfg.init_done) {
        if (ch == '\n') {
            hw_ops->put_char(HW_UART_NO, '\r');
        }
        hw_ops->put_char(HW_UART_NO, ch);
    }

    return;
}

S32 serial_getc(void)
{
    uart_ops *hw_ops = g_sys_serial.hw_ops;

    if (g_sys_serial.cfg.init_done) {
        if (!hw_ops->rx_ready(HW_UART_NO)) {
            return -EAGAIN;
        }

        return hw_ops->get_char(HW_UART_NO);
    }

    return -EAGAIN;
}

void serial_puts(const char *s)
{
    if (!g_sys_serial.cfg.init_done) {
        return;
    }

    while (*s) {
        serial_putc(*s++);
    }

    return;
}

S32 serial_tstc(void)
{
    uart_ops *hw_ops = NULL;

    if (!g_sys_serial.cfg.init_done) {
        return OS_OK;
    }

    hw_ops = g_sys_serial.hw_ops;

    return (S32)hw_ops->rx_ready(HW_UART_NO);
}

void serial_flush(void)
{
    uart_ops *hw_ops = g_sys_serial.hw_ops;

    if (!g_sys_serial.cfg.init_done) {
        return;
    }

    /* just flush, don't care fail */
    (void)hw_ops->wait4idle(g_sys_serial.cfg.hw_uart_no, SERIAL_FLUSH_TMOUT);
}
