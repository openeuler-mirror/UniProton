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

#include "spi_1911.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "prt_gic_external.h"

void IoWrite32(U32 value, void *address)
{
    *(volatile U32 *)address = value;
}

U32 IoRead32(void *address)
{
    return *(volatile U32 *)address;
}

static int SpiCalculateRatio(U32 clkFreq, U32 maxSpeedHz, U32 *divRatioPre, U32 *divRatioPost)
{
    U32 ratio;
    U32 pre = 0;
    U32 post = DIV_RATIO_POST_MAX + 1;
    U32 tmp = 0;

    ratio = clkFreq / maxSpeedHz;
    tmp = clkFreq % maxSpeedHz;
    if ((tmp != 0) || ((ratio & 1) != 0) || (ratio > DIV_RATIO_MAX)) {
        return -1;
    }

    while (post > DIV_RATIO_POST_MAX) {
        pre += DIV_RATIO_PRE;
        if (pre > DIV_RATIO_PRE_MAX) {
            return -1;
        }
        tmp = ratio % pre;
        if (tmp != 0) {
            continue;
        }
        post = ratio / pre - 1;
    }
    *divRatioPre = pre;
    *divRatioPost = post;
    return 0;
}

static int SpiSetCtrl(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiTransInfo *info, U32 clkFreq)
{
    int ret;
    union SpiCommonCtrl cfg;
    U32 divRatioPre;
    U32 divRatioPost;

    cfg.value = 0;
    cfg.bits.frameSize = info->bitsPerWord - 1;
    cfg.bits.loopBack = (info->mode & SPI_LOOP) ? 1 : 0;
    cfg.bits.clkPhase = (info->mode & SPI_CPHA) ? 1 : 0;
    cfg.bits.clkPolarity = (info->mode & SPI_CPOL) ? 1 : 0;

    ret = SpiCalculateRatio(clkFreq, info->maxSpeedHz, &divRatioPre, &divRatioPost);
    if (ret != 0) {
        return ret;
    }

    cfg.bits.divRatioPre = divRatioPre;
    cfg.bits.divRatioPost = divRatioPost;
    IoWrite32(cfg.value, spiBaseAddr->baseAddr + SPI_COMMON_CTRL_REG);
    return 0;
}

static void SpiSetFifoLevel(struct SpiBaseAddrInfo *spiBaseAddr, U32 rxFifoLevel)
{
    union SpiFifoLevelCtrl fifoLevelCtrl;
    fifoLevelCtrl.value = IoRead32(spiBaseAddr->baseAddr + SPI_FIFO_LEVEL_CTRL_REG);
    fifoLevelCtrl.bits.intrLvlRx = rxFifoLevel;
    fifoLevelCtrl.bits.intrLvlTx = SPI_TX_THR_64;
    IoWrite32(fifoLevelCtrl.value, spiBaseAddr->baseAddr + SPI_FIFO_LEVEL_CTRL_REG);
}

static void SpiEnable(struct SpiBaseAddrInfo *spiBaseAddr)
{
    IoWrite32(0xFFFFFFFF, spiBaseAddr->baseAddr + SPI_CS_CTRL_REG);
    IoWrite32(1, spiBaseAddr->baseAddr + SPI_EN_REG);
}

static void SpiDisable(struct SpiBaseAddrInfo *spiBaseAddr)
{
    IoWrite32(0, spiBaseAddr->baseAddr + SPI_CS_CTRL_REG);
    IoWrite32(0, spiBaseAddr->baseAddr + SPI_EN_REG);
}

static void SpiEnableInt(struct SpiBaseAddrInfo *spiBaseAddr)
{
    IoWrite32(~IMR_MASK, spiBaseAddr->baseAddr + INTR_SPI_MASK_REG);
}

static void SpiDisableInt(struct SpiBaseAddrInfo *spiBaseAddr)
{
    IoWrite32(IMR_MASK, spiBaseAddr->baseAddr + INTR_SPI_MASK_REG);
}

static void SpiWriteTx(struct SpiBaseAddrInfo *spiBaseAddr, U32 value)
{
    IoWrite32(value, spiBaseAddr->baseAddr + SPI_DIN_REG);
}

static U32 SpiReadRx(struct SpiBaseAddrInfo *spiBaseAddr)
{
    U32 data;
    data = IoRead32(spiBaseAddr->baseAddr + SPI_DOUT_REG);
    return data;
}

static void SpiGetTransStatus(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiTransStatus *status)
{
    union SpiState temp;
    temp.value = IoRead32(spiBaseAddr->baseAddr + SPI_STATE_REG);
    status->busy = temp.bits.busy;
    status->rxFull = temp.bits.rxFifoFull;
    status->rxNotEmpty = temp.bits.rxFifoNotEmpty;
    status->txNotFull = temp.bits.txFifoNotFull;
    status->txEmpty = temp.bits.txFifoEmpty;
}

static void SpiGetTransIrq(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiTransIrq *status)
{
    union SpiIntrState temp;
    temp.value = IoRead32(spiBaseAddr->baseAddr + INTR_SPI_REG);
    status->txIrq = temp.bits.intrTx;
    status->rxIrq = temp.bits.intrRx;
    status->rxOverflow = temp.bits.intrRxOverflow;
    status->rxTimeout = temp.bits.intrRxTimeout;
}

static void SpiGetFifoInfo(struct SpiBaseAddrInfo *spiBaseAddr, struct SpiFifo *fifo)
{
    union SpiFifoLevelCtrl temp;
    temp.value = IoRead32(spiBaseAddr->baseAddr + SPI_FIFO_LEVEL_CTRL_REG);
    fifo->intrLvlTx = temp.bits.intrLvlTx;
    fifo->intrLvlRx = temp.bits.intrLvlRx;
    fifo->dmaBurstLvlTx = temp.bits.dmaBurstLvlTx;
    fifo->dmaBurstLvlRx = temp.bits.dmaBurstLvlRx;
}

static bool SpiTxNotFull(struct SpiBaseAddrInfo *spiBaseAddr)
{
    union SpiState status;
    status.value = IoRead32(spiBaseAddr->baseAddr + SPI_STATE_REG);
    return (status.bits.txFifoNotFull == 1);
}

static bool SpiRxNotEmpty(struct SpiBaseAddrInfo *spiBaseAddr)
{
    union SpiState status;
    status.value = IoRead32(spiBaseAddr->baseAddr + SPI_STATE_REG);
    return (status.bits.rxFifoNotEmpty == 1);
}

static bool SpiBusy(struct SpiBaseAddrInfo *spiBaseAddr)
{
    union SpiState status;
    status.value = IoRead32(spiBaseAddr->baseAddr + SPI_STATE_REG);
    return (status.bits.busy == 1);
}

static void SpiFlushRxFifo(struct SpiBaseAddrInfo *spiBaseAddr)
{
    U64 limit = TIMEOUT_LIMIT;

    do {
        while (SpiRxNotEmpty(spiBaseAddr)) {
            SpiReadRx(spiBaseAddr);
        }
    } while (SpiBusy(spiBaseAddr) && limit--);
}

static void SpiClearRxoi(struct SpiBaseAddrInfo *spiBaseAddr)
{
    IoWrite32(1, spiBaseAddr->baseAddr + INTR_SPI_CLR_REG);
}

static U32 SpiGetFifoDepth(void)
{
    return FIFO_DEPTH;
}

static void SpiConfigGpio(struct SpiMapGpioInfo *gpio)
{
    IoWrite32(1, gpio->gpioBaseAddr + gpio->gpioOffset0);
    IoWrite32(1, gpio->gpioBaseAddr + gpio->gpioOffset1);
    IoWrite32(1, gpio->gpioBaseAddr + gpio->gpioOffset2);
    IoWrite32(1, gpio->gpioBaseAddr + gpio->gpioOffset3);
}

static int SpiCheckSpiInfo(struct SpiDeviceInfo *spi)
{
    if ((spi->info.maxSpeedHz < SPEED_HZ_MIN) || (spi->info.maxSpeedHz > SPEED_HZ_MAX)) {
        return -1;
    }

    if ((spi->info.bitsPerWord < WIDTH_8_BITS) || (spi->info.bitsPerWord > WIDTH_32_BITS)) {
        return -1;
    }

    if (spi->info.chipSelect >= CS_MAX_NUM) {
        return -1;
    }

    return 0;
}

static int SpiSetup(struct SpiDeviceInfo *spi)
{
    int ret;

    ret = SpiCheckSpiInfo(spi);
    if (ret != 0) {
        return ret;
    }

    SpiDisable(&spi->spiBaseAddr);
    ret = SpiSetCtrl(&spi->spiBaseAddr, &spi->info, spi->clkFreq);
    if (ret != 0) {
        return ret;
    }

    SpiSetFifoLevel(&spi->spiBaseAddr, spi->rxFifoLevel);
    return 0;
}

const struct SpiDeviceOps spiDeviceOps = {
    .Setup = SpiSetup,
    .Enable = SpiEnable,
    .Disable = SpiDisable,
    .EnableInt = SpiEnableInt,
    .DisableInt = SpiDisableInt,
    .WriteTx = SpiWriteTx,
    .ReadRx = SpiReadRx,
    .GetTransStatus = SpiGetTransStatus,
    .GetTransIrq = SpiGetTransIrq,
    .GetFifoInfo = SpiGetFifoInfo,
    .TxNotFull = SpiTxNotFull,
    .RxNotEmpty = SpiRxNotEmpty,
    .FlushRxFifo = SpiFlushRxFifo,
    .ClearRxoi = SpiClearRxoi,
    .GetFifoDepth = SpiGetFifoDepth,
    .ConfigGpio = SpiConfigGpio,
};

/* Atlas 200I DK A2支持Spi0，Ascend310B支持Spi5 */
struct SpiDeviceInfo g_spiDevices[] = {
#if (CONFIG_DEVICE_DK_A2 == YES)
    {
        .spiId = 0,
        .spiBaseAddr.baseAddr = (void *)0x82020000,
        .spiBaseAddr.addrSize = 0x10000,
        .spiOps = &spiDeviceOps,
        .irq = 193,
        .info.maxSpeedHz = SPEED_HZ_MAX,
        .numCs = 1,
        .clkFreq = 150000000,
        .isNeedConfGpio = 0,
    },
#else
    {
        .spiId = 5,
        .spiBaseAddr.baseAddr = (void *)0xC40B0000,
        .spiBaseAddr.addrSize = 0x10000,
        .gpioInfo.gpioBaseAddr = (void *)0xC4000000,
        .gpioInfo.gpioOffset0 = 0xD4,
        .gpioInfo.gpioOffset1 = 0xD8,
        .gpioInfo.gpioOffset2 = 0xDC,
        .gpioInfo.gpioOffset3 = 0xE0,
        .spiOps = &spiDeviceOps,
        .irq = 195,
        .info.maxSpeedHz = SPEED_HZ_MAX,
        .numCs = 1,
        .clkFreq = 150000000,
        .isNeedConfGpio = 1,
    },
#endif
};

U32 g_spiDevicesNum = sizeof(g_spiDevices) / sizeof(struct SpiDeviceInfo);

struct SpiDeviceInfo *FindSpiDeviceById(U32 spiId)
{
    for (U32 i = 0; i < g_spiDevicesNum; i++) {
        if (g_spiDevices[i].spiId == spiId) {
            return &(g_spiDevices[i]);
        }
    }

    return NULL;
}

U32 OsSpiInit(void)
{
    U32 ret;

    for (U32 i = 0; i < g_spiDevicesNum; i++) {
        /* 配置GPIO口复用 */
        if (g_spiDevices[i].isNeedConfGpio) {
            g_spiDevices[i].spiOps->ConfigGpio(&g_spiDevices[i].gpioInfo);
        }

        ret = PRT_HwiSetAttr(g_spiDevices[i].irq, 10, OS_HWI_MODE_ENGROSS);
        if (ret != OS_OK) {
            return ret;
        }

        ret = PRT_HwiCreate(g_spiDevices[i].irq, (HwiProcFunc)SpiIrqHandler, g_spiDevices[i].spiId);
        if (ret != OS_OK) {
            return ret;
        }

        ret = PRT_HwiEnable(g_spiDevices[i].irq);
        if (ret != OS_OK) {
            return ret;
        }

        OsGicdCfgTargetId(g_spiDevices[i].irq, OsGetCoreID());
    }

    return 0;
}