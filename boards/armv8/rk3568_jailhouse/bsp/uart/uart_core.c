#include "uart_core.h"
#include "common.h"

#define UART_REG_READ(addr)          (*(volatile U32 *)(((uintptr_t)addr)))
#define UART_REG_WRITE(value, addr)  (*(volatile U32 *)((uintptr_t)addr) = (U32)value)

/* Get UART register base address */
static S32 uart_core_base_addr(S32 uartno, U32 *reg_base)
{
    unsigned long reg_bases[] = {
        UART0_REG_BASE,
        UART1_REG_BASE,
        UART2_REG_BASE,
        UART3_REG_BASE,
        UART4_REG_BASE
    };

    if (uartno >= MAX_UART_NUM || uartno < 0) {
        return -1;
    }

    *reg_base = reg_bases[uartno];

    return OS_OK;
}

S32 uart_reg_read(S32 uartno, U32 offset, U32 *val)
{
    S32 ret;
    U32 reg_base = 0x0;

    ret = uart_core_base_addr(uartno, &reg_base);
    if (ret) {
        return ret;
    }

    *val = UART_REG_READ((unsigned long)(reg_base + offset));
    return OS_OK;
}

void uart_reg_write(S32 uartno, U32 offset, U32 val)
{
    S32 ret;
    U32 reg_base = 0x0;

    ret = uart_core_base_addr(uartno, &reg_base);
    if (ret) {
        return;
    }

    UART_REG_WRITE(val, (unsigned long)(reg_base + offset));
    return;
}

void uart_set_lcr_dlab(S32 uartno, S32 dlab_sel)
{
    S32 ret;
    U32 lcr = 0;

    ret = uart_reg_read(uartno, DW_UART_LCR, &lcr);
    if (ret) {
        return;
    }

    if (dlab_sel) {
        lcr |= DW_UART_DLAB;
    } else {
        lcr &= ~DW_UART_DLAB;
    }

    uart_reg_write(uartno, DW_UART_LCR, lcr);
    return;
}

void uart_set_dll_dlh(S32 uartno, U32 dll, U32 dlh)
{
    /* Enable DLL/DLH/FCR access */
    uart_set_lcr_dlab(uartno, 1);
    uart_reg_write(uartno, DW_UART_DLL, dll);
    uart_reg_write(uartno, DW_UART_DLH, dlh);
    uart_set_lcr_dlab(uartno, 0);
    return;
}

void uart_get_dll_dlh(S32 uartno, U32 *dll, U32 *dlh)
{
    /* Enable DLL/DLH/FCR access */
    uart_set_lcr_dlab(uartno, 1);
    uart_reg_read(uartno, DW_UART_DLL, dll);
    uart_reg_read(uartno, DW_UART_DLH, dlh);
    uart_set_lcr_dlab(uartno, 0);
    return;
}

void uart_set_fifo_ctrl(S32 uartno, U32 fifo_ctrl)
{
    /* Enable DLL/DLH/FCR access */
    uart_set_lcr_dlab(uartno, 1);
    uart_reg_write(uartno, DW_UART_FCR, fifo_ctrl);
    uart_set_lcr_dlab(uartno, 0);
    return;
}

void uart_set_lcr(S32 uartno, U32 lcr)
{
    uart_reg_write(uartno, DW_UART_LCR, lcr);
    return;
}

/* value: 0：禁止；1：使能 */
void uart_set_irq_enable(S32 uartno, U32 value)
{
    uart_reg_write(uartno, ELSI, value);
    return;
}

S32 uart_is_txfifo_full(S32 uartno)
{
    S32 ret;
    U32 usr = 0;

    ret = uart_reg_read(uartno, DW_UART_USR, &usr);
    if (ret) {
        return OS_OK;
    }

    return (usr & DW_XFIFO_NOT_FULL) ? 0 : 1;
}

S32 uart_is_rx_ready(S32 uartno)
{
    U32 lsr = 0;

    (void)uart_reg_read(uartno, DW_UART_LSR, &lsr);
    return (lsr & DW_UART_LSR_DDR) ? 1 : 0;
}

void uart_tx_char(S32 uartno, const S8 c)
{
    uart_reg_write(uartno, DW_UART_THR, (U32)(U8)c);
    return;
}

void uart_rx_char(S32 uartno, S8 *c)
{
    U32 rbr = 0;

    (void)uart_reg_read(uartno, DW_UART_RBR, &rbr);
    *c = (S8)(rbr & 0xFF);

    return;
}

S32 uart_wait4idle(S32 uartno, U32 timeout)
{
    U32 usr = 0;

    while (timeout) {
        (void)uart_reg_read(uartno, DW_UART_USR, &usr);
        if ((usr & (DW_UART_BUSY)) == 0) {
            return OS_OK;
        }

        timeout--;
    }

    return -1;
}
