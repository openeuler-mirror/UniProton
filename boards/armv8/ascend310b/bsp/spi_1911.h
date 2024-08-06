/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-25
 * Description: SPI功能
 */

#ifndef __SPI_1911_H__
#define __SPI_1911_H__

#include "prt_typedef.h"
#include "prt_config.h"

#define SPI_CS_CTRL_REG 0x000         /* CS control register */
#define SPI_COMMON_CTRL_REG 0x004     /* General control register */
#define SPI_EN_REG 0x008              /* SPI enable register */
#define SPI_FIFO_LEVEL_CTRL_REG 0x00C /* FIFO threshold control register */
#define INTR_SPI_MASK_REG 0x010       /* Interrupt mask register. 1:mask interrupt; 0:not */
#define SPI_DIN_REG 0x014             /* Input data register */
#define SPI_DOUT_REG 0x018            /* Output data register */
#define SPI_STATE_REG 0x01C           /* Status register */
#define INTR_SPI_RAW_REG 0x020        /* Raw interrupt status register. 1:have; 0:not */
#define INTR_SPI_REG 0x024            /* Interrupt status register. 1:have; 0:not */
#define INTR_SPI_CLR_REG 0x028        /* Interrupt clear register */

#define SPI_LOOP (1 << 0)
#define SPI_CPHA (1 << 1)
#define SPI_CPOL (1 << 2)
#define SPI_LSB_FIRST (1 << 3)

#define IMR_RXOF (1 << 0) /* Receive Overflow */
#define IMR_RXTO (1 << 1) /* Receive Timeout */
#define IMR_RX (1 << 2)   /* Receive */
#define IMR_TX (1 << 3)   /* Transmit */
#define IMR_MASK (IMR_TX | IMR_RX | IMR_RXOF)

#define DIV_RATIO_PRE_LAREG 200   /* Large frequency divider */
#define DIV_RATIO_PRE_SMALL_8 8   /* Small frequency divider, applicable to total clock: 200MHz */
#define DIV_RATIO_PRE_SMALL_6 6   /* Small frequency divider, applicable to total clock: 150MHz */
#define SPEED_HZ_THRESHOLD 100000 /* Frequency division system adjustment threshold */
#define DIV_RATIO_MAX 65024       /* Max frequency divider, (DIV_RATIO_POST_MAX + 1) * DIV_RATIO_PRE_MAX */
#define DIV_RATIO_POST_MAX 0xFF
#define DIV_RATIO_PRE_MAX 0xFE
#define DIV_RATIO_PRE 2 /* div_ratio_pre must be even */
#define TIMEOUT_LIMIT (1 << 13)
#define FIFO_DEPTH 256
#define CS_MAX_NUM 1

#define GET_MIN_VALUE(type, a, b) (((type)(a)) < ((type)(b)) ? ((type)(a)) : ((type)(b)))

/* General control register */
union SpiCommonCtrl {
    struct {
        U32 rsv2 : 1;               /* bit:[0] */
        U32 loopBack : 1;           /* bit:[1] */
        U32 clkPolarity : 1;        /* bit:[2] */
        U32 clkPhase : 1;           /* bit:[3] */
        U32 divRatioPre : 8;        /* bit:[11:4] */
        U32 divRatioPost : 8;       /* bit:[19:12] */
        U32 frameSize : 5;          /* bit:[24:20] */
        U32 hiSpd : 1;              /* bit:[25] */
        U32 dmaEnTx : 1;            /* bit:[26] */
        U32 dmaEnRx : 1;            /* bit:[27] */
        U32 timeoutLimitCntSel : 2; /* bit:[29:28] */
        U32 rsv : 2;                /* bit:[31:30] */
    } bits;
    U32 value;
};

/* FIFO threshold control register */
union SpiFifoLevelCtrl {
    struct {
        U32 dmaBurstLvlTx : 3; /* bit:[2:0] */
        U32 intrLvlTx : 3;     /* bit:[5:3] */
        U32 dmaBurstLvlRx : 3; /* bit:[8:6] */
        U32 intrLvlRx : 3;     /* bit:[11:9] */
        U32 rsv : 20;          /* bit:[31:12] */
    } bits;
    U32 value;
};

/* Status register */
union SpiState {
    struct {
        U32 txFifoEmpty : 1;    /* bit:[0] */
        U32 txFifoNotFull : 1;  /* bit:[1] */
        U32 rxFifoNotEmpty : 1; /* bit:[2] */
        U32 rxFifoFull : 1;     /* bit:[3] */
        U32 busy : 1;           /* bit:[4] */
        U32 rsv : 27;           /* bit:[31:5] */
    } bits;
    U32 value;
};

/* Interrupt status register. 1:have; 0:not */
union SpiIntrState {
    struct {
        U32 intrRxOverflow : 1; /* bit:[0] */
        U32 intrRxTimeout : 1;  /* bit:[1] */
        U32 intrRx : 1;         /* bit:[2] */
        U32 intrTx : 1;        /* bit:[3] */
        U32 tfEccMultiErr : 1;  /* bit:[4] */
        U32 tfEcc1BitErr : 1;   /* bit:[5] */
        U32 rfEccMultiErr : 1;  /* bit:[6] */
        U32 rfEcc1BitErr : 1;   /* bit:[7] */
        U32 rsv : 24;           /* bit:[31:8] */
    } bits;
    U32 value;
};

#define WIDTH_32_BITS 32U
#define WIDTH_16_BITS 16U
#define WIDTH_8_BITS 8U
#define CLK_FRE_DEFAULT 200000000
#define SPEED_HZ_MAX 25000000
#define SPEED_HZ_MIN 4000

/* 单位为字节 */
enum SpiRxThr {
    SPI_RX_THR_255 = 0,
    SPI_RX_THR_252,
    SPI_RX_THR_248,
    SPI_RX_THR_240,
    SPI_RX_THR_224,
    SPI_RX_THR_192,
    SPI_RX_THR_128,
    SPI_RX_THR_32,
};

/* 单位为字节 */
enum SpiTxThr {
    SPI_TX_THR_1 = 0,
    SPI_TX_THR_4,
    SPI_TX_THR_8,
    SPI_TX_THR_16,
    SPI_TX_THR_32,
    SPI_TX_THR_64,
    SPI_TX_THR_128,
};

struct SpiTransInfo {
    const void *txBuf;
    void *rxBuf;
    U32 txCnt;
    U32 rxCnt;
    U32 maxSpeedHz;
    U32 mode;
    U8 isLsbFirst;
    U8 bitsPerWord;
    U8 chipSelect;
};

struct SpiBaseAddrInfo {
    void *baseAddr;
    U32 addrSize;
};

/* spi引脚复用的GPIO的信息 */
struct SpiMapGpioInfo {
    void *gpioBaseAddr;
    U8 gpioOffset0;
    U8 gpioOffset1;
    U8 gpioOffset2;
    U8 gpioOffset3;
};

struct SpiTransStatus {
    U8 busy;
    U8 rxFull;
    U8 rxNotEmpty;
    U8 txNotFull;
    U8 txEmpty;
    U8 res[3];
};

struct SpiTransIrq {
    U8 txIrq;
    U8 rxIrq;
    U8 rxOverflow;
    U8 rxTimeout;
};

struct SpiFifo {
    U32 intrLvlTx;
    U32 intrLvlRx;
    U32 dmaBurstLvlTx;
    U32 dmaBurstLvlRx;
};

struct TransferDataInfo {
    const void *txBuf;
    void *rxBuf;
    U32 len;
    U32 bitsPerWord;
    U32 mode;
    U32 chipSelect;
    U32 rxFifoLevel;
};

struct SpiDeviceInfo {
    struct SpiBaseAddrInfo spiBaseAddr;
    struct SpiMapGpioInfo gpioInfo;
    const struct SpiDeviceOps *spiOps;
    struct SpiTransInfo info;
    int irq;
    int numCs;
    U32 spiId;
    U32 fifoDepth;
    U32 clkFreq;
    U32 rxFifoLevel;
    U32 isNeedConfGpio;
};

struct SpiDeviceOps {
    int (*Setup)(struct SpiDeviceInfo *spi);
    void (*Enable)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*Disable)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*EnableInt)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*DisableInt)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*WriteTx)(struct SpiBaseAddrInfo *spiBaseAddr, U32 data);
    U32 (*ReadRx)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*GetTransStatus)(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiTransStatus *status);
    void (*GetTransIrq)(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiTransIrq *status);
    void (*GetFifoInfo)(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiFifo *fifo);
    bool (*RxNotEmpty)(struct SpiBaseAddrInfo *spiBaseAddr);
    bool (*TxNotFull)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*FlushRxFifo)(struct SpiBaseAddrInfo *spiBaseAddr);
    void (*ClearRxoi)(struct SpiBaseAddrInfo *spiBaseAddr);
    U32 (*GetFifoDepth)(void);
    void (*ConfigGpio)(struct SpiMapGpioInfo *gpio);
};

extern struct SpiDeviceInfo g_spiDevices[];
extern U32 g_spiDevicesNum;

void SpiIrqHandler(U32 spiId);
struct SpiDeviceInfo *FindSpiDeviceById(U32 spiId);
int SpiTransferProc(U32 spiId, struct TransferDataInfo *transferInfo);
U32 OsSpiInit(void);
void SpiTransferTest(void);

#endif