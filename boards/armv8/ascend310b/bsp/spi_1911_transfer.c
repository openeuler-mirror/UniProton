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

#include "test.h"
#include "timer.h"
#include "prt_task.h"
#include "spi_1911.h"

static U32 SpiDataConvert32(U32 data)
{
    U32 i;
    U32 result = 0;

    for (i = 0; i < WIDTH_32_BITS; i++) {
        result <<= 1;
        if ((data & 1) != 0) {
            result |= 1;
        }
        data >>= 1;
    }

    return result;
}

static U32 SpiDataConvert16(U16 data)
{
    U32 i;
    U16 result = 0;

    for (i = 0; i < WIDTH_16_BITS; i++) {
        result <<= 1;
        if ((data & 1) != 0) {
            result |= 1;
        }
        data >>= 1;
    }

    return result;
}

static U32 SpiDataConvert8(U8 data)
{
    U32 i;
    U8 result = 0;

    for (i = 0; i < WIDTH_8_BITS; i++) {
        result <<= 1;
        if ((data & 1) != 0) {
            result |= 1;
        }
        data >>= 1;
    }

    return result;
}

static void SpiRxProc(struct SpiDeviceInfo *spi)
{
    U32 count;
    U32 data;

    if (spi->info.rxCnt == 0 || spi->info.rxBuf == NULL) {
        spi->info.rxCnt = 0;
        return;
    }

    count = GET_MIN_VALUE(U32, spi->info.rxCnt, spi->fifoDepth);
    while (spi->spiOps->RxNotEmpty(&spi->spiBaseAddr) && count--) {
        data = spi->spiOps->ReadRx(&spi->spiBaseAddr);
        switch (spi->info.bitsPerWord) {
            case WIDTH_8_BITS:
                data = spi->info.isLsbFirst ? SpiDataConvert8((U8)data) : data;
                *(U8 *)(spi->info.rxBuf) = (U8)data;
                break;
            case WIDTH_16_BITS:
                data = spi->info.isLsbFirst ? SpiDataConvert16((U16)data) : data;
                *(U16 *)(spi->info.rxBuf) = (U16)data;
                break;
            case WIDTH_32_BITS:
            default:
                data = spi->info.isLsbFirst ? SpiDataConvert32((U32)data) : data;
                *(U32 *)(spi->info.rxBuf) = (U32)data;
                break;
        }
        spi->info.rxBuf += ((U32)spi->info.bitsPerWord / WIDTH_8_BITS);
        spi->info.rxCnt--;
    }
}

static void SpiTxProc(struct SpiDeviceInfo *spi)
{
    U32 count;
    U32 data = 0;

    if (spi->info.txCnt == 0 || spi->info.txBuf == NULL) {
        spi->info.txCnt = 0;
        return;
    }

    count = GET_MIN_VALUE(U32, spi->info.txCnt, spi->fifoDepth);
    while (spi->spiOps->TxNotFull(&spi->spiBaseAddr) && count--) {
        switch (spi->info.bitsPerWord) {
            case WIDTH_8_BITS:
                data = *(U8 *)(spi->info.txBuf);
                data = spi->info.isLsbFirst ? SpiDataConvert8((U8)data) : data;
                break;
            case WIDTH_16_BITS:
                data = *(U16 *)(spi->info.txBuf);
                data = spi->info.isLsbFirst ? SpiDataConvert16((U16)data) : data;
                break;
            case WIDTH_32_BITS:
            default:
                data = *(U32 *)(spi->info.txBuf);
                data = spi->info.isLsbFirst ? SpiDataConvert32((U32)data) : data;
                break;
        }
        spi->info.txBuf += ((U32)spi->info.bitsPerWord / WIDTH_8_BITS);
        spi->info.txCnt--;
        spi->spiOps->WriteTx(&spi->spiBaseAddr, data);
    }
}

void SpiIrqHandler(U32 spiId)
{
    struct SpiTransIrq spiIrqStatus = { 0 };
    struct SpiDeviceInfo *spi = FindSpiDeviceById(spiId);

    if (spi == NULL) {
        return;
    }

    spi->spiOps->GetTransIrq(&spi->spiBaseAddr, &spiIrqStatus);
    if (spiIrqStatus.rxOverflow) {
        spi->spiOps->FlushRxFifo(&spi->spiBaseAddr);
        spi->spiOps->ClearRxoi(&spi->spiBaseAddr);
        goto finalize_transfer;
    }

    if (spiIrqStatus.rxIrq) {
        SpiRxProc(spi);
    }

    if (spi->info.rxCnt == 0) {
        goto finalize_transfer;
    }

    if (spiIrqStatus.txIrq) {
        SpiTxProc(spi);
    }

    return;

finalize_transfer:
    spi->spiOps->Disable(&spi->spiBaseAddr);
    spi->spiOps->DisableInt(&spi->spiBaseAddr);

    return;
}

static void SpiSaveLoadTransInfo(struct SpiTransInfo *dst, struct SpiTransInfo *src)
{
    dst->txBuf = src->txBuf;
    dst->rxBuf = src->rxBuf;
    dst->txCnt = src->txCnt;
    dst->rxCnt = src->rxCnt;
    dst->maxSpeedHz = src->maxSpeedHz;
    dst->mode = src->mode;
    dst->isLsbFirst = src->isLsbFirst;
    dst->bitsPerWord = src->bitsPerWord;
    dst->chipSelect = src->chipSelect;
}

int SpiTransferProc(U32 spiId, struct TransferDataInfo *transferInfo)
{
    int ret;
    struct SpiTransInfo tmpInfo;
    struct SpiDeviceInfo *spi = FindSpiDeviceById(spiId);

    if (spi == NULL) {
        return -1;
    }

    SpiSaveLoadTransInfo(&tmpInfo, &spi->info);
    spi->info.txBuf = transferInfo->txBuf;
    spi->info.rxBuf = transferInfo->rxBuf;
    spi->info.bitsPerWord = transferInfo->bitsPerWord;
    spi->info.txCnt = transferInfo->len / (transferInfo->bitsPerWord / WIDTH_8_BITS);
    spi->info.rxCnt = spi->info.txCnt;
    spi->info.chipSelect = transferInfo->chipSelect;
    spi->info.mode = transferInfo->mode;
    spi->info.isLsbFirst = ((transferInfo->mode & SPI_LSB_FIRST) != 0);
    spi->fifoDepth = spi->spiOps->GetFifoDepth();

    ret = spi->spiOps->Setup(spi);
    if (ret != 0) {
        SpiSaveLoadTransInfo(&spi->info, &tmpInfo);
        return ret;
    }

    spi->spiOps->FlushRxFifo(&spi->spiBaseAddr);
    __asm__ __volatile__("dmb ish" : : : "memory");
    spi->spiOps->EnableInt(&spi->spiBaseAddr);
    spi->spiOps->Enable(&spi->spiBaseAddr);

    return 0;
}

void SpiTransferTest(void)
{
#if (CONFIG_SPI_ENABLE == YES)
    int ret;
    U8 txBuf[] = "123 Hello world !!! 123 Hello UniProton !!!";
    U8 rxBuf[sizeof(txBuf)];
    struct TransferDataInfo transInfo;
#if (CONFIG_DEVICE_DK_A2 == YES)
    U32 spiId = 0;
#else
    U32 spiId = 5;
#endif

    PRT_TaskDelay(OS_TICK_PER_SECOND);
    PRT_Printf("[uniproton] Tx:%s\n", txBuf);
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    transInfo.txBuf = (void *)txBuf;
    transInfo.rxBuf = (void *)rxBuf;
    transInfo.len = sizeof(txBuf);
    transInfo.bitsPerWord = WIDTH_8_BITS;
    transInfo.mode = SPI_LOOP; /* 环回自验模式 */
    transInfo.chipSelect = 0;
    transInfo.rxFifoLevel = SPI_RX_THR_32;
    ret = SpiTransferProc(spiId, &transInfo);
    if (ret != 0) {
        PRT_Printf("[uniproton] Failed to transfer data by spi[%u], ret:%d\n", spiId, ret);
    }

    PRT_Printf("[uniproton] Rx:%s\n", rxBuf);
#endif
}