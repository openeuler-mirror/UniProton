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
 * Create: 2024-1-15
 * Description: I2C功能
 */

#ifndef __I2C_1911_H__
#define __I2C_1911_H__

#include "ascend310b/cpu_config.h"
#include "prt_config.h"

#define BIT_MASK(last, first) \
    ((0xffffffffffffffffULL >> (64 - ((last) + 1 - (first)))) << (first))

#define DIV_ROUND_UP_ULL(a, b) \
    ((a) / (b) + (((a) % (b) >= 0) ? 1 : 0))

#define FIELD_PREP(mask, val)           \
    ({                                  \
        U32 value = (val);              \
        for (int i = 0; i < 64; i++) {  \
            if ((mask) & (1 << i)) {    \
                break;                  \
            }                           \
            value <<= 1;                \
        }                               \
        (mask) & value;                 \
    })

#define FILED_GET(mask, val)            \
    ({                                  \
        U32 value = (val);              \
        value &= (mask);                \
        for (int i = 0; i < 64; i++) {  \
            if ((mask) & (1 << i)) {    \
                break;                  \
            }                           \
            value >>= 1;                \
        }                               \
        value;                          \
    })

#define I2C_SET_WRADDR(buf, wrAddr)          \
    ({                                       \
        (buf)[0] = (0xff00 & (wrAddr)) >> 8; \
        (buf)[1] = 0xff & (wrAddr);          \
    })

#define I2C_FRAME_CTRL_REG 0x0000
#define I2C_FRAME_CTRL_SPEED_MODE BIT_MASK(1, 0)
#define I2C_FRAME_CTRL_ADDR_TEN BIT(2)

#define I2C_SLV_ADDR_REG 0x0004
#define I2C_SLV_ADDR_VAL BIT_MASK(9, 0)

#define I2C_CMD_TXDATA_REG 0x0008
#define I2C_CMD_TXDATA_DATA BIT_MASK(7, 0)
#define I2C_CMD_TXDATA_RW BIT(8)
#define I2C_CMD_TXDATA_P_EN BIT(9)
#define I2C_CMD_TXDATA_SR_EN BIT(10)

#define I2C_RXDATA_REG 0x000C
#define I2C_SS_SCL_HCNT_REG 0x0010
#define I2C_SS_SCL_LCNT_REG 0x0014
#define I2C_FS_SCL_HCNT_REG 0x0018
#define I2C_FS_SCL_LCNT_REG 0x001C
#define I2C_HS_SCL_HCNT_REG 0x0020
#define I2C_HS_SCL_LCNT_REG 0x0024

#define I2C_FIFO_CTRL_REG 0x0028
#define I2C_FIFO_CTRL_RX_AF_THRESH BIT_MASK(7, 2)
#define I2C_FIFO_CTRL_TX_AE_THRESH BIT_MASK(13, 8)
#define I2C_FIFO_CTRL_RX_CLR BIT(0)
#define I2C_FIFO_CTRL_TX_CLR BIT(1)

#define I2C_FIFO_STATE_REG 0x002C
#define I2C_FIFO_STATE_RX_RERR BIT(0)
#define I2C_FIFO_STATE_RX_WERR BIT(1)
#define I2C_FIFO_STATE_RX_EMPTY BIT(3)
#define I2C_FIFO_STATE_TX_RERR BIT(6)
#define I2C_FIFO_STATE_TX_WERR BIT(7)
#define I2C_FIFO_STATE_TX_FULL BIT(11)

#define I2C_SDA_HOLD_REG 0x0030
#define I2C_SDA_HOLD_TX BIT_MASK(15, 0)

#define I2C_DMA_CTRL_REG 0x0034
#define I2C_FS_SPK_LEN_REG 0x0038
#define I2C_HS_SPK_LEN_REG 0x003C

#define I2C_INT_CLR_REG0 0x0040
#define I2C_INT_CLR_TX_AEMPTY BIT(0)

#define I2C_INT_MSTAT_REG 0x0044
#define I2C_INT_MSTAT_ALL BIT_MASK(4, 0)
#define I2C_INT_MSTAT_TRANS_CPLT BIT(0)
#define I2C_INT_MSTAT_TRANS_ERR BIT(1)
#define I2C_INT_MSTAT_FIFO_ERR BIT(2)
#define I2C_INT_MSTAT_RX_FULL BIT(3)
#define I2C_INT_MSTAT_TX_EMPTY BIT(4)

#define I2C_INT_CLR_REG 0x0048
#define I2C_INT_CLR_TRANS_CPLT BIT(0)
#define I2C_INT_CLR_TRANS_ERR BIT(1)
#define I2C_INT_CLR_FIFO_ERR BIT(2)
#define I2C_INT_CLR_RX_AFULL BIT(3)

#define I2C_INT_MASK_REG 0x004C
#define I2C_TRANS_STATE_REG 0x0050

#define I2C_STD_SPEED_MODE 0
#define I2C_FAST_SPEED_MODE 1
#define I2C_HIGH_SPEED_MODE 2

#define I2C_TX_F_AE_THRESH 1
#define I2C_RX_F_AF_THRESH 60

#define I2C_TX_FIFO_DEPTH 64
#define I2C_RX_FIFO_DEPTH 64

#define I2C_MAX_STD_MODE_FREQ 100000
#define I2C_MAX_FAST_MODE_FREQ 400000
#define I2C_MAX_HIGH_MODE_FREQ 3400000

#define I2C_STD_MODE_DEF_SCL_NS 1000
#define I2C_FAST_MODE_DEF_SCL_NS 300
#define I2C_HIGH_MODE_DEF_SCL_NS 120

#define HZ_PER_KHZ 1000

#define FAST_SPEED_MODE_DIVIDE 26
#define FAST_SPEED_MODE_DIVISOR 76
#define HIGH_SPEED_MODE_DIVIDE 6
#define HIGH_SPEED_MODE_DIVISOR 22
#define STD_SPEED_MODE_DIVIDE 40
#define STD_SPEED_MODE_DIVISOR 87

struct I2cTimings {
    U32 busFreqHz;
    U32 sclRiseNs;
    U32 sclFallNs;
    U32 sclIntDelayNs;
    U32 sdaFallNs;
    U32 sdaHoldNs;
    U32 digitalFilterWidthNs;
    U32 analogFilterCutoffFreqHz;
};

struct I2cTimingsCfg {
    U32 clkRateKHz;
    U32 spkLen;
    U32 sclHcnt;
    U32 sclLcnt;
    U32 sdaHoldCnt;
};

struct I2cTimingsInfo {
    struct I2cTimings t;
    struct I2cTimingsCfg cfg;
};

#define I2C_M_RD 0x0001 /* read data from slave to master */
#define I2C_M_TEN 0x0010 /* ten bit chip address */
struct I2cMsg {
    U16 addr; /* slave address */
    U16 flags;
    U16 len; /* msg length */
    U8 *buf; /* pointer to msg data */
    U32 timeoutMs;
};

#define USEC_PER_MSEC 1000L
#define NSEC_PER_MSEC 1000000L
struct I2cTransInfo {
    int cmdComplete;
    struct I2cMsg *msg;
    int msgNum;
    int msgTxIdx;
    int bufTxIdx;
    int msgRxIdx;
    int bufRxIdx;
    int msgErr;
};

struct I2cIntStatusInfo {
    U8 isTxEmptyIrq;
    U8 isRxFullIrq;
    U8 isTransCpltIrq;
    U8 isErrIrq;
};

struct I2cBaseAddrInfo {
    void *baseAddr;
    U32 size;
};

struct I2cOps {
    void (*ConfigBus)(struct I2cBaseAddrInfo *baseAddrInfo, U32 freqHz, struct I2cTimingsCfg *timingsCfg);
    void (*EnableTxEmptyInt)(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable);
    void (*EnableRxFullInt)(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable);
    void (*EnableTransCpltInt)(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable);
    void (*EnableErrInt)(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable);
    void (*DisableAllInt)(struct I2cBaseAddrInfo *baseAddrInfo);
    void (*GetIntStatusInfo)(struct I2cBaseAddrInfo *baseAddrInfo, struct I2cIntStatusInfo *status);
    void (*ClearTxEmptyInt)(struct I2cBaseAddrInfo *baseAddrInfo);
    void (*ClearRxFullInt)(struct I2cBaseAddrInfo *baseAddrInfo);
    void (*ClearTransCpltInt)(struct I2cBaseAddrInfo *baseAddrInfo);
    void (*ClearErrInt)(struct I2cBaseAddrInfo *baseAddrInfo);
    void (*ClearAllInt)(struct I2cBaseAddrInfo *baseAddrInfo);
    void (*TransferInit)(struct I2cBaseAddrInfo *baseAddrInfo, U16 addr, bool is10BitAddr);
    void (*WriteTxData)(struct I2cBaseAddrInfo *baseAddrInfo, U8 data, U16 isRead, bool needStop, bool needRestart);
    U8 (*ReadRxData)(struct I2cBaseAddrInfo *baseAddrInfo);
    bool (*TxFifoFull)(struct I2cBaseAddrInfo *baseAddrInfo);
    bool (*RxFifoEmpty)(struct I2cBaseAddrInfo *baseAddrInfo);
    U32 (*GetTxFifoLimit)(struct I2cBaseAddrInfo *baseAddrInfo);
};

struct I2cCtrl {
    U32 i2cId;
    int irq;
    struct I2cBaseAddrInfo baseAddrInfo;
    const struct I2cOps *ops;
    struct I2cTransInfo transInfo;
    struct I2cTimingsInfo timingsInfo;
};

void IoWrite32(U32 value, void *address);
U32 IoRead32(void *address);

struct I2cCtrl *FindI2cCtrlById(U32 i2cId);
const struct I2cOps *I2cGetOps(void);
int I2cIrqHandle(int I2cId);
int I2cMasterTransfer(U32 i2cId, struct I2cMsg *msg, int num);

U32 I2cInit(void);
void I2cTransferTest(void);

#endif