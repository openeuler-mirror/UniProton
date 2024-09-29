/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-06-20
 * Description: SPI功能
 */

#ifndef __SPI_H__
#define __SPI_H__

#include "prt_typedef.h"
#include "prt_config.h"

#define LOG_WRITE_BASE 0x51788000ULL
#define LOG_HERE(offset) ((*(volatile U32 *)(LOG_WRITE_BASE + (offset))) = 0x47474747)
#define LOG_HERE_WITH_VAL(offset, val) ((*(volatile U32 *)(LOG_WRITE_BASE + (offset))) = (val))

#define SUBCTRL_REG_BASE 0x201070000
#define SC_SPI_RESET_DREQ_REG 0xa64
#define SC_SPI_MUX_REG 0x2000

#define CPLD_BASE 0x80000000
#define ENABLE_SPI_OFFSET 0x20

#define IOMUX_REG_BASE 0x201100000      /* GPIO Config register */
#define GPIO38_REG 0x64
#define GPIO41_REG 0x70
#define GPIO02_REG 0x7C
#define GPIO03_REG 0x80
#define GPIO05_REG 0x88

typedef enum {
    MOTOROLA = 0,
    TISSP,
    NAMICROWIRE,
} SpiFrameType;

#define SPI_FRAME_LEN_8_BIT 7
#define SPI_FRAME_LEN_16_BIT 15

#define SPI_CLOCK_EDGE_UP 0
#define SPI_CLOCK_EDGE_DOWN 1

#define SPI_CLOCK_POLARITY_LOW 0
#define SPI_CLOCK_POLARITY_HIGH 1

typedef struct {
    U8 port;             /* 总线0-1 */
    U8 dev;              /* 从机选择 */
    U8 loopBack;         /* 环回口 */
    U32 baud_rate;       /* 波特率 */
    U32 frameType;       /* 帧类型 */
    U32 frameLength;     /* 帧长度 */
    U32 clockEdge;       /* 时钟相位 */
    U32 clockPolarity;   /* 时钟极性 */
} SpiAttr;

#define SPI_CLK_DIV 10
#define SPI_IMR_TXEIM (0x1U << 0)
#define SPI_IMR_RXFIM (0x1U << 4)
#define SPI_IMR_MSTIM (0x1U << 5)
#define SPI_IMR_MASK (SPI_IMR_TXEIM | SPI_IMR_RXFIM | SPI_IMR_MSTIM)
#define SPI_TX_FIFO_THRE 8
#define SPI_RX_FIFO_THRE 8
#define SPI_TX_FIFO_LEVEL 32
#define SPI_RX_FIFO_LEVEL 32

#define SPI_REG_BASE 0x2011A0000
#define SPI_CTRLR0_REG      0x00
#define SPI_CTRLR1_REG      0x04
#define SPI_SSIENR_REG      0x08
#define SPI_SER_REG         0x10
#define SPI_BAUDR_REG       0x14
#define SPI_TXFTLR_REG      0x18
#define SPI_RXFTLR_REG      0x1C
#define SPI_TXFLR_REG       0x20
#define SPI_RXFLR_REG       0x24
#define SPI_SR_REG          0x28
#define SPI_IMR_REG         0x2C
#define SPI_ISR_REG         0x30
#define SPI_SISR_REG        0x34
#define SPI_ICR_REG         0x48
#define SPI_DR0_REG         0x60
#define RX_SAMPLE_DLY_REG   0xF0

#define GPIO0_REG_BASE 0x201120000
#define GPIO1_REG_BASE 0x201130000
#define GPIO_SWPORT_DR_REG 0x0
#define GPIO_SWPORT_DDR_REG 0x4
#define GPIO_PER_REG_NUM 32
#define GPIO_MAX_NUM 64
#define GPIO_LEVEL_LOW 0
#define GPIO_LEVEL_HIGH 1
#define SPI_CS_GPIO_N 0x3
#define SPI_TPM_RST 0x3
#define GPIO38 38
#define GPIO41 41
#define SPI_SYSTEM_CLOCK 250000000
#define LOOP_TEST_FRAME_NUM 4

#define LOOP_ENABLE 0x1
#define LOOP_DISABLE 0x0
#define SPI_CS_LOW 0
#define SPI_CS_HIGH 1
#define SPI_RECV_FIFO_EMPTY 0
#define SPI_RECV_FIFO_NOT_EMPTY 1
#define FRAME_LEN_TO_MASK(frame_len) ((0x1<<(frame_len+1)) - 1)

typedef union {
    U32 value;
    struct {
        U32 reserved1   : 4;
        U32 frf         : 2;
        U32 scph        : 1;
        U32 scpol       : 1;
        U32 tmod        : 2;
        U32 reserved2   : 1;
        U32 srl         : 1;
        U32 cfs         : 4;
        U32 dfs32       : 5;
        U32 reserved3   : 11;
    } bits;
} SpiCtrlr0;

#define SPI_IRQ 491
#define SPI_IRQ_PRIOR 10

#define SPI_ISR_TXEIS SPI_IMR_TXEIM
#define SPI_ISR_RXFIS SPI_IMR_RXFIM
#define SPI_ISR_MSTIS SPI_IMR_MSTIM

typedef struct {
    U8 dev;
    U64 txAddr;
    U8 *txBuf;
    U8 *rxBuf;
    U8 *txValue;
    U32 txBufLen;
    U32 rxBufLen;
    U32 txValueLen;
    U32 frameLength;
} SpiTransInfo;

#define SPI_FRAME_HEADER_BYTE_NUM 4

typedef struct {
    U8 body[SPI_FRAME_HEADER_BYTE_NUM];
} SpiFrameHeader;

#define SPI_TRANS_MODE_DUPLEX       0
#define SPI_TRANS_MODE_SEND         1
#define SPI_TRANS_MODE_RECEIVE      2
#define SPI_TRANS_MODE_EEPROM_RD    3

#define SPI_START_RECEIVE 0x5A5A

#define SPI_SR_RFNE (0x1U << 3)
#define SPI_SR_TFE (0x1U << 2)
#define SPI_SR_BUSY (0x1U << 0)

#define SPI_CHECKBUSY_TIMEOUT_LIMIT (0x1U << 13)

#define SPI_DEFAULT_ID 0

#endif