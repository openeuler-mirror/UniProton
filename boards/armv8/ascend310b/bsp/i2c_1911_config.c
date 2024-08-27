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

#include "prt_typedef.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_gic_external.h"
#include "i2c_1911.h"

void IoRegWrite32(U32 value, void *address)
{
    *(volatile U32 *)address = value;
}

U32 IoRegRead32(void *address)
{
    return *(volatile U32 *)address;
}

void I2cSetSpeedMode(struct I2cBaseAddrInfo *baseAddrInfo, U32 speedMode)
{
    U32 val = IoRegRead32(baseAddrInfo->baseAddr + I2C_FRAME_CTRL_REG);

    val &= ~I2C_FRAME_CTRL_SPEED_MODE;
    val |= FIELD_PREP(I2C_FRAME_CTRL_SPEED_MODE, speedMode);
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_FRAME_CTRL_REG);
}

void I2cSetTxSdaHold(struct I2cBaseAddrInfo *baseAddrInfo, U32 sdaHoldCnt)
{
    U32 val = IoRegRead32(baseAddrInfo->baseAddr + I2C_SDA_HOLD_REG);

    val &= ~I2C_SDA_HOLD_TX;
    val |= FIELD_PREP(I2C_SDA_HOLD_TX, sdaHoldCnt);
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_SDA_HOLD_REG);
}

void I2cSetFifoThreshold(struct I2cBaseAddrInfo *baseAddrInfo)
{
    U32 val = IoRegRead32(baseAddrInfo->baseAddr + I2C_FIFO_CTRL_REG);

    val &= ~I2C_FIFO_CTRL_RX_AF_THRESH;
    val &= ~I2C_FIFO_CTRL_TX_AE_THRESH;
    val |= FIELD_PREP(I2C_FIFO_CTRL_RX_AF_THRESH, I2C_RX_F_AF_THRESH);
    val |= FIELD_PREP(I2C_FIFO_CTRL_TX_AE_THRESH, I2C_TX_F_AE_THRESH);
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_FIFO_CTRL_REG);
}

void I2cConfigBus(struct I2cBaseAddrInfo *baseAddrInfo, U32 freqHz, struct I2cTimingsCfg *timingsCfg)
{
    U32 speedMode;

    switch (freqHz) {
        case I2C_MAX_FAST_MODE_FREQ:
            speedMode = I2C_FAST_SPEED_MODE;
            IoRegWrite32(timingsCfg->sclHcnt, baseAddrInfo->baseAddr + I2C_FS_SCL_HCNT_REG);
            IoRegWrite32(timingsCfg->sclLcnt, baseAddrInfo->baseAddr + I2C_FS_SCL_LCNT_REG);
            break;
        case I2C_MAX_HIGH_MODE_FREQ:
            speedMode = I2C_HIGH_SPEED_MODE;
            IoRegWrite32(timingsCfg->sclHcnt, baseAddrInfo->baseAddr + I2C_HS_SCL_HCNT_REG);
            IoRegWrite32(timingsCfg->sclLcnt, baseAddrInfo->baseAddr + I2C_HS_SCL_LCNT_REG);
            break;
        case I2C_MAX_STD_MODE_FREQ:
            /* fall through */
        default:
            speedMode = I2C_STD_SPEED_MODE;
            IoRegWrite32(timingsCfg->sclHcnt, baseAddrInfo->baseAddr + I2C_SS_SCL_HCNT_REG);
            IoRegWrite32(timingsCfg->sclLcnt, baseAddrInfo->baseAddr + I2C_SS_SCL_LCNT_REG);
            break;
    }

    I2cSetSpeedMode(baseAddrInfo, speedMode);
    I2cSetTxSdaHold(baseAddrInfo, timingsCfg->sdaHoldCnt);
    IoRegWrite32(timingsCfg->spkLen, baseAddrInfo->baseAddr + I2C_FS_SPK_LEN_REG);
    I2cSetFifoThreshold(baseAddrInfo);

}

void I2cEnableTxEmptyInt(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable)
{
    U32 intMask = IoRegRead32(baseAddrInfo->baseAddr + I2C_INT_MASK_REG);

    if (enable) {
        intMask |= I2C_INT_MSTAT_TX_EMPTY;
    } else {
        intMask &= ~I2C_INT_MSTAT_TX_EMPTY;
    }
    IoRegWrite32(intMask, baseAddrInfo->baseAddr + I2C_INT_MASK_REG);
}

void I2cEnableRxFullInt(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable)
{
    U32 intMask = IoRegRead32(baseAddrInfo->baseAddr + I2C_INT_MASK_REG);

    if (enable) {
        intMask |= I2C_INT_MSTAT_RX_FULL;
    } else {
        intMask &= ~I2C_INT_MSTAT_RX_FULL;
    }
    IoRegWrite32(intMask, baseAddrInfo->baseAddr + I2C_INT_MASK_REG);
}

void I2cEnableTransCpltInt(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable)
{
    U32 intMask = IoRegRead32(baseAddrInfo->baseAddr + I2C_INT_MASK_REG);

    if (enable) {
        intMask |= I2C_INT_MSTAT_TRANS_CPLT;
    } else {
        intMask &= ~I2C_INT_MSTAT_TRANS_CPLT;
    }
    IoRegWrite32(intMask, baseAddrInfo->baseAddr + I2C_INT_MASK_REG);
}

void I2cEnableErrInt(struct I2cBaseAddrInfo *baseAddrInfo, U32 enable)
{
    U32 intMask = IoRegRead32(baseAddrInfo->baseAddr + I2C_INT_MASK_REG);

    if (enable) {
        intMask |= I2C_INT_MSTAT_TRANS_ERR;
    } else {
        intMask &= ~I2C_INT_MSTAT_TRANS_ERR;
    }
    IoRegWrite32(intMask, baseAddrInfo->baseAddr + I2C_INT_MASK_REG);
}

void I2cDisableAllInt(struct I2cBaseAddrInfo *baseAddrInfo)
{
    IoRegWrite32(0, baseAddrInfo->baseAddr + I2C_INT_MASK_REG);
}

void I2cGetIntStatusInfo(struct I2cBaseAddrInfo *baseAddrInfo, struct I2cIntStatusInfo *status)
{
    U32 intStat = IoRegRead32(baseAddrInfo->baseAddr + I2C_INT_MSTAT_REG);

    if (intStat & I2C_INT_MSTAT_TX_EMPTY) {
        status->isTxEmptyIrq = 1;
    }

    if (intStat & I2C_INT_MSTAT_TRANS_ERR) {
        status->isErrIrq = 1;
    }

    if (intStat & I2C_INT_MSTAT_RX_FULL) {
        status->isRxFullIrq = 1;
    }

    if (intStat & I2C_INT_MSTAT_TRANS_CPLT) {
        status->isTransCpltIrq = 1;
    }
}

void I2cClearTxEmptyInt(struct I2cBaseAddrInfo *baseAddrInfo)
{
    IoRegWrite32(I2C_INT_CLR_TX_AEMPTY, baseAddrInfo->baseAddr + I2C_INT_CLR_REG0);
}

void I2cClearRxFullInt(struct I2cBaseAddrInfo *baseAddrInfo)
{
    IoRegWrite32(I2C_INT_CLR_RX_AFULL, baseAddrInfo->baseAddr + I2C_INT_CLR_REG);
}

void I2cClearTransCpltInt(struct I2cBaseAddrInfo *baseAddrInfo)
{
    IoRegWrite32(I2C_INT_CLR_TRANS_CPLT, baseAddrInfo->baseAddr + I2C_INT_CLR_REG);
}

void I2cClearErrInt(struct I2cBaseAddrInfo *baseAddrInfo)
{
    IoRegWrite32(I2C_INT_CLR_TRANS_ERR, baseAddrInfo->baseAddr + I2C_INT_CLR_REG);
}

void I2cClearAllInt(struct I2cBaseAddrInfo *baseAddrInfo)
{
    IoRegWrite32(I2C_INT_MSTAT_ALL, baseAddrInfo->baseAddr + I2C_INT_CLR_REG);
    IoRegWrite32(I2C_INT_CLR_TX_AEMPTY, baseAddrInfo->baseAddr + I2C_INT_CLR_REG0);
}

void SetSlaveAddr(struct I2cBaseAddrInfo *baseAddrInfo, U16 addr, bool is10BitAddr)
{
    U32 val = IoRegRead32(baseAddrInfo->baseAddr + I2C_FRAME_CTRL_REG);

    val &= ~I2C_FRAME_CTRL_ADDR_TEN;
    if (is10BitAddr) {
        val |= I2C_FRAME_CTRL_ADDR_TEN;
    }
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_FRAME_CTRL_REG);

    val = IoRegRead32(baseAddrInfo->baseAddr + I2C_SLV_ADDR_REG);
    val &= ~I2C_SLV_ADDR_VAL;
    val |= FIELD_PREP(I2C_SLV_ADDR_VAL, addr);
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_SLV_ADDR_REG);
}

void ClearTxRxFifo(struct I2cBaseAddrInfo *baseAddrInfo)
{
    U32 val = IoRegRead32(baseAddrInfo->baseAddr + I2C_FIFO_CTRL_REG);

    val |= I2C_FIFO_CTRL_TX_CLR | I2C_FIFO_CTRL_RX_CLR;
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_FIFO_CTRL_REG);
    
    val &= ~(I2C_FIFO_CTRL_TX_CLR | I2C_FIFO_CTRL_RX_CLR);
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_FIFO_CTRL_REG);
}

void I2cTransferInit(struct I2cBaseAddrInfo *baseAddrInfo, U16 addr, bool is10BitAddr)
{
    SetSlaveAddr(baseAddrInfo, addr, is10BitAddr);
    ClearTxRxFifo(baseAddrInfo);
}

void I2cWriteTxData(struct I2cBaseAddrInfo *baseAddrInfo, U8 data, U16 isRead, bool needStop, bool needRestart)
{
    U32 val = 0;

    if (needRestart) {
        val |= I2C_CMD_TXDATA_SR_EN;
    }

    if (needStop) {
        val |= I2C_CMD_TXDATA_P_EN;
    }

    if (isRead) {
        val |= I2C_CMD_TXDATA_RW;
    } else {
        val &= (~I2C_CMD_TXDATA_RW);
        val |= FIELD_PREP(I2C_CMD_TXDATA_DATA, data);
    }
    IoRegWrite32(val, baseAddrInfo->baseAddr + I2C_CMD_TXDATA_REG);
}

U8 I2cReadRxData(struct I2cBaseAddrInfo *baseAddrInfo)
{
    return (U8)IoRegRead32(baseAddrInfo->baseAddr + I2C_RXDATA_REG);
}

bool I2cTxFifoFull(struct I2cBaseAddrInfo *baseAddrInfo)
{
    U32 fifoState = IoRegRead32(baseAddrInfo->baseAddr + I2C_FIFO_STATE_REG);

    if (fifoState & I2C_FIFO_STATE_TX_FULL) {
        return true;
    }

    return false;
}

bool I2cRxFifoEmpty(struct I2cBaseAddrInfo *baseAddrInfo)
{
    U32 fifoState = IoRegRead32(baseAddrInfo->baseAddr + I2C_FIFO_STATE_REG);

    if (fifoState & I2C_FIFO_STATE_RX_EMPTY) {
        return true;
    }

    return false;
}

U32 I2cGetTxFifoLimit(struct I2cBaseAddrInfo *baseAddrInfo)
{
    U32 val, txThresh;

    val = IoRegRead32(baseAddrInfo->baseAddr + I2C_FIFO_CTRL_REG);
    txThresh = FILED_GET(I2C_FIFO_CTRL_TX_AE_THRESH, val);
    return (I2C_TX_FIFO_DEPTH - txThresh);
}

const struct I2cOps g_i2cOps = {
    .ConfigBus = I2cConfigBus,
    .DisableAllInt = I2cDisableAllInt,
    .EnableTxEmptyInt = I2cEnableTxEmptyInt,
    .EnableRxFullInt = I2cEnableRxFullInt,
    .EnableTransCpltInt = I2cEnableTransCpltInt,
    .EnableErrInt = I2cEnableErrInt,
    .ClearAllInt = I2cClearAllInt,
    .ClearTxEmptyInt = I2cClearTxEmptyInt,
    .ClearRxFullInt = I2cClearRxFullInt,
    .ClearTransCpltInt = I2cClearTransCpltInt,
    .ClearErrInt = I2cClearErrInt,
    .WriteTxData = I2cWriteTxData,
    .ReadRxData = I2cReadRxData,
    .GetIntStatusInfo = I2cGetIntStatusInfo,
    .TxFifoFull = I2cTxFifoFull,
    .RxFifoEmpty = I2cRxFifoEmpty,
    .TransferInit = I2cTransferInit,
    .GetTxFifoLimit = I2cGetTxFifoLimit,
};

const struct I2cOps *I2cGetOps(void)
{
    return &g_i2cOps;
}

struct I2cCtrl g_i2cCtrls[] = {
    {
        .i2cId = 8,
        .baseAddrInfo.baseAddr = (void *)0x82070000,
        .baseAddrInfo.size = 0x10000,
        .irq = 188,
        .timingsInfo.t.sclFallNs = 0,
        .timingsInfo.t.sdaHoldNs = 0x14D,
        .timingsInfo.t.sclRiseNs = 0xA6,
        .timingsInfo.t.digitalFilterWidthNs = 0x21,
        .timingsInfo.t.busFreqHz = I2C_MAX_STD_MODE_FREQ,
        .timingsInfo.cfg.clkRateKHz = 150000000 / HZ_PER_KHZ,
    },
};

U32 g_i2cCtrlsNum = sizeof(g_i2cCtrls) / sizeof(struct I2cCtrl);

struct I2cCtrl *FindI2cCtrlById(U32 i2cId)
{
    for (U32 i = 0; i < g_i2cCtrlsNum; i++) {
        if (g_i2cCtrls[i].i2cId == i2cId) {
            return &(g_i2cCtrls[i]);
        }
    }

    return NULL;
}

void I2cCalcuCfgCnt(struct I2cTimings *t, struct I2cTimingsCfg *cfg)
{
    U32 totalCnt;
    U32 sclHcnt, sclLcnt, sclFallCnt, sclRiseCnt;
    U32 divide, divisor;

    switch (t->busFreqHz) {
        case I2C_MAX_FAST_MODE_FREQ:
            divide = FAST_SPEED_MODE_DIVIDE;
            divisor = FAST_SPEED_MODE_DIVISOR;
            break;
        case I2C_MAX_HIGH_MODE_FREQ:
            divide = HIGH_SPEED_MODE_DIVIDE;
            divisor = HIGH_SPEED_MODE_DIVISOR;
            break;
        case I2C_MAX_STD_MODE_FREQ:
            /* fall through */
        default:
            t->busFreqHz = I2C_MAX_STD_MODE_FREQ;
            divide = STD_SPEED_MODE_DIVIDE;
            divisor = STD_SPEED_MODE_DIVISOR;
            break;
    }

    totalCnt = DIV_ROUND_UP_ULL(cfg->clkRateKHz * HZ_PER_KHZ, t->busFreqHz);
    sclHcnt = DIV_ROUND_UP_ULL(totalCnt * divide, divisor);
    sclLcnt = totalCnt - sclHcnt;
    sclFallCnt = DIV_ROUND_UP_ULL(t->sclFallNs * cfg->clkRateKHz, NSEC_PER_MSEC);
    sclRiseCnt = DIV_ROUND_UP_ULL(t->sclRiseNs * cfg->clkRateKHz, NSEC_PER_MSEC);
    cfg->sdaHoldCnt = DIV_ROUND_UP_ULL(t->sdaHoldNs * cfg->clkRateKHz, NSEC_PER_MSEC);
    cfg->spkLen = DIV_ROUND_UP_ULL(t->digitalFilterWidthNs * cfg->clkRateKHz, NSEC_PER_MSEC);
    cfg->sclHcnt = sclHcnt - cfg->spkLen - 7 - sclFallCnt;
    cfg->sclLcnt = sclLcnt - 1 - sclRiseCnt;
}

U32 I2cInit(void)
{
    U32 ret;

    for (U32 i = 0; i < g_i2cCtrlsNum; i++) {
        struct I2cCtrl *i2cCtrl = &(g_i2cCtrls[i]);

        i2cCtrl->transInfo.cmdComplete = 0;
        i2cCtrl->ops = I2cGetOps();

        i2cCtrl->ops->DisableAllInt(&i2cCtrl->baseAddrInfo);
        i2cCtrl->ops->ClearAllInt(&i2cCtrl->baseAddrInfo);

        I2cCalcuCfgCnt(&i2cCtrl->timingsInfo.t, &i2cCtrl->timingsInfo.cfg);
        i2cCtrl->ops->ConfigBus(&i2cCtrl->baseAddrInfo, i2cCtrl->timingsInfo.t.busFreqHz, &i2cCtrl->timingsInfo.cfg);

        ret = PRT_HwiSetAttr(i2cCtrl->irq, 10, OS_HWI_MODE_ENGROSS);
        if (ret != OS_OK) {
            return ret;
        }

        ret = PRT_HwiCreate(i2cCtrl->irq, (HwiProcFunc)I2cIrqHandle, i2cCtrl->i2cId);
        if (ret != OS_OK) {
            return ret;
        }

        ret = PRT_HwiEnable(i2cCtrl->irq);
        if (ret != OS_OK) {
            return ret;
        }

        OsGicdCfgTargetId(i2cCtrl->irq, OsGetCoreID());
    }

    return OS_OK;
}