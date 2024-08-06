#include "serial.h"
#include "uart_core.h"
#include "uart_regs.h"
#include "common.h"
#include "prt_typedef.h"

static S32 uart_busy_timeout(void)
{
    return UART_BUSY_TIMEOUT;
}

/* Caculate UART Divisor Factor */
void calc_uart_dll_dlh(U32 uartclk, U32 baudrate, U32 *dll, U32 *dlh)
{
    U32 divisor;

    /*
     * baudrate = bus_clock / (16 * divisor)
     * ==> divisor = bus_clock/(baudrate * 16) */
    DIV_ROUND_CLOSEST(uartclk, (16 * baudrate), divisor); /* 16 just for codestyle */
    *dll = divisor & 0xFF;
    *dlh = (divisor >> 8) & 0xFF; /* take high 8 bits */
}

static U32 calc_lcr_reg_val(serial_cfg *cfg)
{
    U32 lcr = 0;

    switch (cfg->data_bits) {
        case 8: /* 8个数据位 */
            lcr |= DW_UART_8bit;
            break;
        case 7: /* 7个数据位 */
            lcr |= DW_UART_7bit;
            break;
        case 6: /* 6个数据位 */
            lcr |= DW_UART_6bit;
            break;
        case 5: /* 5个数据位 */
            lcr |= DW_UART_5bit;
            break;
        default:
            lcr |= DW_UART_8bit;
            break;
    }

    /* 0 - 1 stop bit 1 - 2 stop bit */
    if (cfg->stop == 2) {
        lcr |= DW_UART_STOP;
    }

    if (cfg->pen) {
        lcr |= DW_UART_PEN;
        if (cfg->eps) {
            lcr |= DW_UART_EPS;
        }
    }

    return lcr;
}

void uart_set_baudrate(serial_cfg *cfg)
{
    U32 dll = 0;
    U32 dlh = 0;

    /* Wait till uart is idle */
    (void)uart_wait4idle(cfg->hw_uart_no, uart_busy_timeout());
    calc_uart_dll_dlh(cfg->uart_src_clk, cfg->baud_rate, &dll, &dlh);
    uart_set_dll_dlh(cfg->hw_uart_no, dll, dlh);
}

S32 uart_init(serial_cfg *cfg)
{
    U32 fifo_ctrl;

    /* max data_bits is 8 */
    if ((cfg == NULL) || (cfg->hw_uart_no >= MAX_UART_NUM) || (cfg->hw_uart_no < 0) || (cfg->data_bits > 8)) {
        return -EINVAL;
    }

    /* Config FIFO,DLL,DLH at first */
    uart_set_baudrate(cfg);
    fifo_ctrl = FIFOENA | UART_FCR_RXCLR | UART_FCR_TXCLR;
    uart_set_fifo_ctrl(cfg->hw_uart_no, fifo_ctrl);
    /* Set data bits, stop bit, parity check */
    uart_set_lcr(cfg->hw_uart_no, calc_lcr_reg_val(cfg));

    /* enable rx irq */
    uart_set_irq_enable(cfg->hw_uart_no, 1);
    return OS_OK;
}

S32 uart_tx(S32 hw_uart_no, const char c)
{
    S32 timeout = 0;
    S32 max_timeout = uart_busy_timeout();

    while (uart_is_txfifo_full(hw_uart_no)) {
        timeout++;
        if (timeout >= max_timeout) {
            return -ETIME;
        }
    }

    uart_tx_char(hw_uart_no, c);
    return OS_OK;
}

S32 uart_rx(S32 hw_uart_no)
{
    S32 timeout = 0;
    S32 max_timeout = uart_busy_timeout();
    S32 c = 0;

    while (!uart_is_rx_ready(hw_uart_no)) {
        timeout++;
        if (timeout >= max_timeout) {
            return -ETIME;
        }
    }

    uart_rx_char(hw_uart_no, (S8 *)&c);
    return c;
}

void uart_raw_puts(S32 hw_uart_no, const S8 *str)
{
    while (*str != '\0') {
        uart_tx_char(hw_uart_no, *str++);
    }
}

S32 uart_tx_ready(S32 hw_uart_no)
{
    if (uart_is_txfifo_full(hw_uart_no)) {
        return OS_OK;
    } else {
        return 1;
    }
}

S32 uart_rx_ready(S32 hw_uart_no)
{
    return uart_is_rx_ready(hw_uart_no);
}

/*
 * Wait until uart is not busy
 * Return 0 if success,otherwise return -1
 */
S32 uart_busy_poll(S32 hw_uart_no)
{
    return uart_wait4idle(hw_uart_no, uart_busy_timeout());
}

serial_cfg g_uart_cfg = {
    .name = "uart",
    .data_bits = 8, /* default data_bits is 8 */
    .stop = 1,
    .pen = 0,
    .baud_rate = 115200 /* default baud_rate is 115200 */
};

uart_ops g_uart_ops = {
    .init = uart_init,
    .setbrg = uart_set_baudrate,
    .put_char = uart_tx,
    .get_char = uart_rx,
    .tx_ready = uart_tx_ready,
    .rx_ready = uart_rx_ready,
    .wait4idle = uart_wait4idle,
};
