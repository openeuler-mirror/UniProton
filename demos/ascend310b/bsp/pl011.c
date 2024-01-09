#include "pl011.h"
#include "cpu_config.h"
#include "prt_gic_external.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "test.h"
#include "ymodem.h"
#include "prt_queue.h"
#include "prt_sem.h"

static U32 g_uartQueue;

static inline void UartSetBaudrate()
{
    U32 baudRate;
    U32 divider;
    U32 remainder;
    U32 fraction;

    baudRate  = PL011_CLK_DIV * CONSOLE_UART_BAUDRATE;
    divider   = UART_CLK_INPUT / baudRate;
    remainder = UART_CLK_INPUT % baudRate;
    baudRate  = (PL011_NUM_8 * remainder) / CONSOLE_UART_BAUDRATE;
    fraction  = (baudRate >> 1) + (baudRate & 1);

    UART_REG(UART_IBRD) = divider;
    UART_REG(UART_FBRD) = fraction;
}

void PRT_UartInit(void)
{
    /* First, disable everything */
    UART_REG(UART_CR) = 0;

    /* set Scale factor of baud rate */
    UartSetBaudrate();

    /* Set the UART to be 8 bits, 1 stop bit, no parity, fifo enabled. */
    UART_REG(UART_LCR_H) = UART_LCR_H_8_BIT | UART_LCR_H_FIFO_EN;

    /* enable the UART */
    UART_REG(UART_CR) = UART_CR_EN | UART_CR_TX_EN | UART_CR_RX_EN;
}

static void UartInterruptHandler(void)
{
    U32 ret;
    if ((UART_REG(UART_MIS) & UART_RTRIS) == UART_RTRIS) {
        UART_REG(UART_ICR) |= UART_RTRIS;
        while (!(UART_REG(UART_FR) & UART_RXFE)) {
            char ch = UART_REG(UART_DR);
            ret = PRT_QueueWrite(g_uartQueue, &ch, sizeof(char), 0, OS_QUEUE_NORMAL);
            if (ret != 0) {
                PRT_Printf("write queue fail %u\n", ret);
            }
        }
        return;
    }

    UART_REG(UART_ICR) |= UART_RXRIS;
    while (!(UART_REG(UART_FR) & UART_RXFE)) {
        char ch = UART_REG(UART_DR);
        ret = PRT_QueueWrite(g_uartQueue, &ch, sizeof(char), 0, OS_QUEUE_NORMAL);
        if (ret != 0) {
            PRT_Printf("write queue fail %u\n", ret);
        }
    }
}

 U32 PRT_UartInterruptInit(void)
 {
    UART_REG(UART_ILPR) = 16;
    UART_REG(UART_IFLS) = UART_INT_HALF;
    UART_REG(UART_IMSC) |= UART_RXRIS | UART_RTRIS;

    U32 ret = PRT_QueueCreate(255, UART_QUEUE_SIZE, &g_uartQueue);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiSetAttr(UART_INT_NUM, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiCreate(UART_INT_NUM, (HwiProcFunc)UartInterruptHandler, 0);
    if (ret != OS_OK) {
        return ret;
    }

    ret = PRT_HwiEnable(UART_INT_NUM);
    if (ret != OS_OK) {
        return ret;
    }

    OsGicdCfgTargetId(UART_INT_NUM, OsGetCoreID());
    return 0;
 }

void UartPutChar(unsigned char ch)
{
    while (UART_REG(UART_FR) & UART_TXFF) {
        asm volatile("yield" ::: "memory");
    }
    UART_REG(UART_DR) = ch;
}

U32 UartGetChar(unsigned char *ch, U32 timeout)
{
    if (ch == NULL) {
        return 1;
    }
    U32 len = 1;
    return PRT_QueueRead(g_uartQueue, ch, &len, timeout);
}
