/*
 * Copyright (c) HiSilicon (Shanghai) Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

#include "bm_common.h"
#include "sample_common.h"
#include "bm_uart.h"
#include "bm_dmac.h"
#include "securec.h"

#define SMPLE_UART_IRQ_CORE 3
#define SAMPLE_UART_NUM 4
#define SAMPLE_UART4_INT 92
#define SAMPLE_UART4_PRIO 6
#define SAMPLE_SEND_LEN 128
#define SAMPLE_RECV_LEN 4
#define SAMPLE_UART_RECV_TIMES 6
#define SAMPLE_UART_DELAY 100
#define SAMPLE_UART_DMA_TEST 0

#if defined(SAMPLE_UART_DMA_TEST) && SAMPLE_UART_DMA_TEST == 1
XFER_DATA char g_buff_tx[SAMPLE_SEND_LEN] = {0};
XFER_DATA char g_buff_rx[SAMPLE_RECV_LEN] = {0};

void mcs_peripherals_shutdown(void)
{
#if defined(__BAREMETAL__)
    bm_disable_irq(SAMPLE_UART4_INT);
#if defined(SAMPLE_UART_DMA_TEST) && SAMPLE_UART_DMA_TEST == 1
    bm_disable_irq(DMAC_INTID);
#endif
#endif
    return;
}

static void uart_recv_async_callback(bm_uart_num uart_id, bm_uart_transmit_async_t type)
{
    bm_log("bm_uart_reacv_async_t = %d, %d\n", uart_id, type);
    bm_log("%s\n", g_buff_rx);
}

static void uart_send_async_callback(bm_uart_num uart_id, bm_uart_transmit_async_t type)
{
    bm_log("bm_uart_send_async_t = %d, %d\n", uart_id, type);
}

static int sample_uart_dma(void)
{
    int uart_num = SAMPLE_UART_NUM;
    int ret;
    serial_cfg uart_cfg = {
        .hw_uart_no = uart_num,
        .uart_src_clk = UART_CLK,
        .data_bits = UART_DATA_8BIT, /* default data_bits is 8 */
        .stop = UART_STOP_1BIT,
        .pen = UART_VERIFY_DISABLE,
        .baud_rate = 115200 /* default baud_rate is 115200 */
    };
    bm_dmac_init();
    ret = bm_uart_init(&uart_cfg);
    if (ret) {
        return BM_FAIL;
    }
    for (int i = 0; i < SAMPLE_RECV_LEN; i++) {
        g_buff_rx[i] = 0;
    }
    for (int i = 0; i < SAMPLE_SEND_LEN; i++) {
        g_buff_tx[i] = 'a' + i % ('z' - 'a' + 1);
    }
    ret = bm_uart_tx_dma(uart_num, g_buff_tx, SAMPLE_SEND_LEN, uart_send_async_callback);
    if (ret) {
        return BM_FAIL;
    }
    ret = bm_uart_rx_dma(uart_num, g_buff_rx, SAMPLE_RECV_LEN, uart_recv_async_callback);
    return ret;
}

#elif defined(SAMPLE_UART_DMA_TEST) && SAMPLE_UART_DMA_TEST == 0
/* ************ uart send sample start ************ */

XFER_DATA char *sample_uart_str = "sample_uart: uart4 send by core";
static int sample_uart_send(unsigned int core)
{
    int uart_num = SAMPLE_UART_NUM;
    serial_cfg uart_cfg = {
        .hw_uart_no = uart_num,
        .uart_src_clk = UART_CLK,
        .data_bits = UART_DATA_8BIT, /* default data_bits is 8 */
        .stop = UART_STOP_1BIT,
        .pen = UART_VERIFY_DISABLE,
        .baud_rate = 115200 /* default baud_rate is 115200 */
    };
    int ret;
    ret = bm_uart_init(&uart_cfg);
    if (ret) {
        bm_log("bm_uart_init failed\n");
        return BM_FAIL;
    }

    ret = bm_uart_raw_puts(uart_num, sample_uart_str, SAMPLE_SEND_LEN);
    if (ret) {
        bm_log("bm_uart_raw_puts failed\n");
        return BM_FAIL;
    }
    ret = bm_uart_tx(uart_num, (char)(core + '0'));
    if (ret) {
        bm_log("bm_uart_tx failed\n");
        return BM_FAIL;
    }
    ret = bm_uart_tx(uart_num, '\n');

    return ret;
}
/* ************ uart send sample end ************ */
/* ************ uart interrupt receive sample start ************ */
static void sample_uart4_recv_isr(void *arg)
{
    char data;
    int ret;
    static int count = 0;
    ret = bm_uart_rx(SAMPLE_UART_NUM, &data);
    if (ret == BM_OK) {
        bm_log("sample_uart: [arg:%d] uart4 interrupt rxdata[%c]\n", (int)(uintptr_t)arg, data);
        count++;
    } else {
        bm_log("sample_uart: uart4 interrupt rx failed\n");
    }
    if (count > SAMPLE_UART_RECV_TIMES) {
        bm_uart_set_irq_enable(SAMPLE_UART_NUM, 0);
#if defined(__BAREMETAL__)
        bm_free_irq(SAMPLE_UART_NUM);
#elif defined(__UNIPROTON__)
        PRT_HwiDelete(SAMPLE_UART_NUM);
#endif
    }
    return;
}
static int sample_uart_interrupt_receive(unsigned int core)
{
    int uart_num = SAMPLE_UART_NUM;
    serial_cfg uart_cfg = {
        .uart_src_clk = UART_CLK,
        .data_bits = UART_DATA_8BIT, /* default data_bits is 8 */
        .stop = UART_STOP_1BIT,
        .pen = UART_VERIFY_DISABLE,
        .hw_uart_no = uart_num,
        .baud_rate = 115200 /* default baud_rate is 115200 */
    };

    bm_uart_init(&uart_cfg);
    bm_uart_set_irq_enable(uart_num, 1);

#if defined(__BAREMETAL__)
    /* route */
    if (core == SMPLE_UART_IRQ_CORE) {
        bm_irq_set_priority(SAMPLE_UART4_INT, SAMPLE_UART4_PRIO);
        bm_irq_set_affinity(SAMPLE_UART4_INT, core);
        bm_request_irq(SAMPLE_UART4_INT, sample_uart4_recv_isr, NULL);
        bm_enable_irq(SAMPLE_UART4_INT);
    }
#elif defined(__UNIPROTON__)
    /* route */
    if (core == SMPLE_UART_IRQ_CORE) {
        PRT_HwiSetAttr(SAMPLE_UART4_INT, SAMPLE_UART4_PRIO, OS_HWI_MODE_ENGROSS);
        PRT_HwiCreate(SAMPLE_UART4_INT, (HwiProcFunc)(uintptr_t)sample_uart4_recv_isr, 0);
        PRT_HwiSetAffinity(SAMPLE_UART4_INT, 1 << core);
        PRT_HwiEnable(SAMPLE_UART4_INT);
    }
#endif
    return BM_OK;
}
/* ************ uart interrupt receive sample end ************ */

/* ************ uart polling receive sample start ************ */
static int sample_uart_polling_receive(unsigned int core)
{
    (void)core;
    int ret;
    int count = 0;
    int uart_num = SAMPLE_UART_NUM;
    serial_cfg uart_cfg = {
        .hw_uart_no = uart_num,
        .uart_src_clk = UART_CLK,
        .data_bits = UART_DATA_8BIT, /* default data_bits is 8 */
        .stop = UART_STOP_1BIT,
        .pen = UART_VERIFY_DISABLE,
        .baud_rate = 115200 /* default baud_rate is 115200 */
    };

    ret = bm_uart_init(&uart_cfg);
    if (ret) {
        return BM_FAIL;
    }

    ret = bm_uart_set_irq_enable(uart_num, 1);
    if (ret) {
        return BM_FAIL;
    }

    char data;
    while (1) {
        if (bm_uart_rx(SAMPLE_UART_NUM, &data) == BM_OK) {
            bm_log("sample_uart: uart4 polling rxdata[%c]\n", data);
            count++;
        }
        if (count > SAMPLE_UART_RECV_TIMES) {
            break;
        }
        udelay(50000); /* delay 50000us */
    }
    return BM_OK;
}
#endif

/* ************ uart polling receive sample end ************ */

/* ************ sample_uart start ************ */
#if defined(__BAREMETAL__)
int main(void)
#elif defined(__UNIPROTON__)
int app_main(void)
#endif
{
    PRT_Printf("app_main\n");
    sample_prepare();
    unsigned int uart_core = bm_get_coreid();
    switch (uart_core) {
        case SAMPLE_CORE0:
            return BM_OK;
        case SAMPLE_CORE1:
            break;
        case SAMPLE_CORE2:
            break;
        case SAMPLE_CORE3:
#if defined(SAMPLE_UART_DMA_TEST) && SAMPLE_UART_DMA_TEST == 1
            sample_uart_dma();
#elif defined(SAMPLE_UART_DMA_TEST) && SAMPLE_UART_DMA_TEST == 0
            sample_uart_send(uart_core);
            sample_uart_interrupt_receive(uart_core);
            sample_uart_polling_receive(uart_core);
            sample_uart_interrupt_receive(uart_core);
            bm_log("sample_uart success\n");
#endif
            break;
        default:
            bm_log("sample_uart: invalid core_num\n");
            break;
    }
    while (1) {
    }
    return BM_OK;
}
/* ************ sample_uart end ************ */
