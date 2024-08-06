/*
 * Copyright (c) Hisilicon Technologies Co., Ltd. 2024-2024. All rights reserved.
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 * 	http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 */
#ifndef __BM_UART_H__
#define __BM_UART_H__
/**
 * @defgroup bm_uart bm_uart
 */

#define BM_OPEN_UART_DEVICE 0

#define PERI_APB_FREQ 100000000
#define UART_CLK (PERI_APB_FREQ)

#define BM_MAX_SHOW_LEN 0x200

#define CORE_SYS_UART2_INTID 90
#define CORE_SYS_UART3_INTID 91
#define CORE_SYS_UART4_INTID 92
#define CORE_SYS_UART5_INTID 93
#define CORE_SYS_UART6_INTID 94
#define CORE_SYS_UART7_INTID 95
#define CORE_SYS_UART_INTID_BASE CORE_SYS_UART2_INTID

typedef enum {
    UART_NUM2 = 2,
    UART_NUM3,
    UART_NUM4,
    UART_NUM5,
    UART_NUM6,
    UART_NUM7,
    UART_NUM_BUTT,
} bm_uart_num;

#define UART_MIN_NUM UART_NUM2
#define UART_MAX_NUM UART_NUM7

typedef enum {
    UART_DATA_5BIT = 0,
    UART_DATA_6BIT,
    UART_DATA_7BIT,
    UART_DATA_8BIT,
    UART_DATA_BIT_BUTT,
} bm_uart_data_bits;

typedef enum {
    UART_STOP_1BIT = 0,
    UART_STOP_1P5BIT,
    UART_STOP_2BIT,
    UART_STOP_BIT_BUTT,
} bm_uart_stop_bits;

typedef enum {
    UART_VERIFY_DISABLE = 0,
    UART_VERIFY_ENABLE,
    UART_VERIFY_BUTT,
} bm_uart_pen;

typedef enum {
    UART_VERIFY_ODD = 0,
    UART_VERIFY_EVEN,
    UART_VERIFY_MODE_BUTT,
} bm_uart_eps;

typedef enum {
    UART_STATUS_DATA_READY = 1 << 0,
    UART_STATUS_OVERFLOWING_ERR = 1 << 1,
    UART_STATUS_VERIFY_ERR = 1 << 2,
    UART_STATUS_FRAME_ERR = 1 << 3,
    UART_STATUS_BREAK_INTERRUPT = 1 << 4,
    UART_STATUS_SEND_FIFO_NULL = 1 << 5,
    UART_STATUS_SEND_REG_FIFO_NULL = 1 << 6,
    UART_STATUS_RECIVE_FIFO_ERR = 1 << 7,
} bm_uart_status;

typedef struct serial_cfg_stuct {
    bm_uart_num hw_uart_no;
    bm_uart_data_bits data_bits;
    bm_uart_stop_bits stop;
    bm_uart_pen pen;
    bm_uart_eps eps;
    unsigned int uart_src_clk;
    unsigned int baud_rate;
} serial_cfg;

typedef enum {
    UART_WRITE_OK = 0,
    UART_WRITE_FAIL,
    UART_READ_OK,
    UART_READ_FAIL,
    UART_TRANSMIT_BUTT,
} bm_uart_transmit_async_t;

typedef void (*bm_uart_callback)(bm_uart_num, bm_uart_transmit_async_t);
/**
 *
 * @ingroup bm_uart
 * @brief Initialize the uart.
 *
 * @par description:
 * Initialize the uart.
 *
 * @param cfg [in]: Pointer to the uart configuration structure
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_init(serial_cfg *cfg);

/**
 *
 * @ingroup bm_uart
 * @brief deinitialize the uart.
 *
 * @par description:
 * deinitialize the uart.
 *
 * @param uart_no [in]: uart number
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_deinit(bm_uart_num uart_no);

/**
 *
 * @ingroup bm_uart
 * @brief Set the current uart to the print uart.
 *
 * @par description:
 * Set the current uart to the print uart.
 *
 * @param uart_no [in]: uart number
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_enable_print(bm_uart_num uart_no);

/**
 *
 * @ingroup bm_uart
 * @brief uart printf.
 *
 * @par description:
 * uart printf.
 *
 * @param format [in]: string
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_printf(const char *format, ...);

/**
 *
 * @ingroup bm_uart
 * @brief enbale uart interrupt.
 *
 * @par description:
 * enbale uart interrupt.
 *
 * @param uart_no [in]: uart number
 * @param status [in]: 1/0, enable/disable uart interrupt
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_set_irq_enable(bm_uart_num uart_no, unsigned int status);

/**
 *
 * @ingroup bm_uart
 * @brief uart send a char.
 *
 * @par description:
 * uart send a char.
 *
 * @param uart_no [in]: uart number
 * @param c [in]: send char
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_tx(bm_uart_num uart_no, const char c);

/**
 *
 * @ingroup bm_uart
 * @brief uart receive a byte.
 *
 * @par description:
 * uart receive a byte.
 *
 * @param uart_no [in]: uart number
 * @param out     [out]: uart rx char*
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_rx(bm_uart_num uart_no, char* out);

/**
 *
 * @ingroup bm_uart
 * @brief uart send a string.
 *
 * @par description:
 * uart send a string.
 *
 * @param uart_no [in]: uart number
 * @param str [in]: send string pointer
 * @param len [in]: send string length
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_raw_puts(bm_uart_num uart_no, const char *str, unsigned int len);

/**
 *
 * @ingroup bm_uart
 * @brief get uart status.
 *
 * @par description:
 * get uart status.
 *
 * @param uart_no [in]: uart number
 * @param status  [out]: status pointer
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_get_status(bm_uart_num uart_no, bm_uart_status *status);

/**
 *
 * @ingroup bm_uart
 * @brief uart receive bytes use dma.
 *
 * @par description:
 * uart receive bytes use dma.
 *
 * @param uart_no [in]: uart number
 * @param out [in]: receive string pointer
 * @param len [in]: receive string length
 * @param callback [in]: bm_uart_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_rx_dma(bm_uart_num uart_no, char* out, unsigned int len, bm_uart_callback callback);

/**
 *
 * @ingroup bm_uart
 * @brief uart send a string use dma.
 *
 * @par description:
 * uart send a string use dma.
 *
 * @param uart_no [in]: uart number
 * @param str [in]: send string pointer
 * @param len [in]: send string length
 * @param callback [in]: bm_uart_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_uart_tx_dma(bm_uart_num uart_no, const char *str, unsigned int len, bm_uart_callback callback);
#endif /* __BM_UART_H__ */