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

#ifndef __BM_DMAC_H__
#define __BM_DMAC_H__

/**
 * @defgroup bm_dmac bm_dmac
 */

#define DMAC_INTID 148

typedef enum {
    DMAC_CLOCK_OPEN = 0,
    DMAC_CLOCK_CLOSE,
    DMAC_CLOCK_BUTT,
} bm_dmac_clock_t;

typedef enum {
    DMAC_TRANSMIT_OK = 0,
    DMAC_TRANSMIT_FAIL,
    DMAC_TRANSMIT_BUTT,
} bm_dmac_transmit_async_t;

typedef enum {
    DMAC_REQ_SEL_SPI0_TX = 0,
    DMAC_REQ_SEL_SPI0_RX,
    DMAC_REQ_SEL_I2C0_TX,
    DMAC_REQ_SEL_I2C0_RX,
    DMAC_REQ_SEL_I2C1_TX,
    DMAC_REQ_SEL_I2C1_RX,
    DMAC_REQ_SEL_UART2_TX,
    DMAC_REQ_SEL_UART2_RX,
    DMAC_REQ_SEL_UART3_TX,
    DMAC_REQ_SEL_UART3_RX,
    DMAC_REQ_SEL_UART4_TX,
    DMAC_REQ_SEL_UART4_RX,
    DMAC_REQ_SEL_I2C2_TX,
    DMAC_REQ_SEL_I2C2_RX,
    DMAC_REQ_SEL_I2C3_TX,
    DMAC_REQ_SEL_I2C3_RX,
    DMAC_REQ_SEL_I2C4_TX,
    DMAC_REQ_SEL_I2C4_RX,
    DMAC_REQ_SEL_I2C5_TX,
    DMAC_REQ_SEL_I2C5_RX,
    DMAC_REQ_SEL_I2C6_TX,
    DMAC_REQ_SEL_I2C6_RX,
    DMAC_REQ_SEL_I2C7_TX,
    DMAC_REQ_SEL_I2C7_RX,
    DMAC_REQ_SEL_I2C8_TX,
    DMAC_REQ_SEL_I2C8_RX,
    DMAC_REQ_SEL_I2C9_TX,
    DMAC_REQ_SEL_I2C9_RX,
    DMAC_REQ_SEL_I2C10_TX,
    DMAC_REQ_SEL_I2C10_RX,
    DMAC_REQ_SEL_I2C11_TX,
    DMAC_REQ_SEL_I2C11_RX,
    DMAC_REQ_SEL_I2C12_TX,
    DMAC_REQ_SEL_I2C12_RX,
    DMAC_REQ_SEL_I2C13_TX,
    DMAC_REQ_SEL_I2C13_RX,
    DMAC_REQ_SEL_I2C14_TX,
    DMAC_REQ_SEL_I2C14_RX,
    DMAC_REQ_SEL_I2C15_TX,
    DMAC_REQ_SEL_I2C15_RX,
    DMAC_REQ_SEL_UART0_TX,
    DMAC_REQ_SEL_UART0_RX,
    DMAC_REQ_SEL_UART1_TX,
    DMAC_REQ_SEL_UART1_RX,
    DMAC_REQ_SEL_UART5_TX,
    DMAC_REQ_SEL_UART5_RX,
    DMAC_REQ_SEL_UART6_TX,
    DMAC_REQ_SEL_UART6_RX,
    DMAC_REQ_SEL_UART7_TX,
    DMAC_REQ_SEL_UART7_RX,
    DMAC_REQ_SEL_SPI1_TX,
    DMAC_REQ_SEL_SPI1_RX,
    DMAC_REQ_SEL_BUTT,
} bm_dmac_req;

typedef enum transfer_type_t {
    TRASFER_TYPE_M2M = 0,
    TRASFER_TYPE_M2P,
    TRASFER_TYPE_P2M,
    TRASFER_TYPE_BUTT,
} transfer_type;

typedef void (*bm_dmac_callback)(bm_dmac_req, bm_dmac_transmit_async_t);

typedef struct {
    bm_dmac_req req;
    unsigned int src_addr;
    unsigned int dest_addr;
    unsigned int data_len;
    transfer_type trans_type;
} bm_dmac_channel_cfg;

/**
 *
 * @ingroup bm_dmac
 * @brief DMAC init.
 *
 * @par description:
 * DMAC init.
 *
 * @param void
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_dmac_init(void);

/**
 *
 * @ingroup bm_dmac
 * @brief DMAC deinit.
 *
 * @par description:
 * DMAC deinit.
 *
 * @param void
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_dmac_deinit(void);

/**
 *
 * @ingroup bm_dmac
 * @brief DMAC clock config.
 *
 * @par description:
 * DMAC clock config.
 *
 * @param cfg [in]: bm_dmac_clock_t
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_dmac_config_io_clock(bm_dmac_clock_t clock);

/**
 *
 * @ingroup bm_dmac
 * @brief DMAC channel configuration and startup sync.
 *
 * @par description:
 * DMAC channel configuration and startup sync.
 *
 * @param channel_cfg [in]: bm_dmac_channel_cfg
 * @param time_wait(ms) [in]: int
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_dmac_transmit_sync(const bm_dmac_channel_cfg *channel_cfg, int time_wait);

/**
 *
 * @ingroup bm_dmac
 * @brief DMAC channel configuration and startup async.
 *
 * @par description:
 * DMAC channel configuration and startup async.
 *
 * @param channel_cfg [in]: bm_dmac_channel_cfg
 * @param callback [in]: bm_dmac_callback
 *
 * @return BM_OK/BM_FAIL
 *
 */
int bm_dmac_transmit_async(const bm_dmac_channel_cfg *channel_cfg, bm_dmac_callback callback);

#endif /* __BM_DMAC_H__ */