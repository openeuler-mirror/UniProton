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

#include <string.h>
#include "prt_typedef.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "timer.h"
#include "prt_gic_external.h"
#include "i2c_1911.h"

void I2cClearTransInfo(struct I2cCtrl *i2cCtrl)
{
    memset(&(i2cCtrl->transInfo), 0, sizeof(struct I2cTransInfo));
}

void I2cEnableTransInt(struct I2cCtrl *i2cCtrl, U32 enable)
{
    i2cCtrl->ops->EnableTxEmptyInt(&i2cCtrl->baseAddrInfo, enable);
    i2cCtrl->ops->EnableRxFullInt(&i2cCtrl->baseAddrInfo, enable);
    i2cCtrl->ops->EnableTransCpltInt(&i2cCtrl->baseAddrInfo, enable);
    i2cCtrl->ops->EnableErrInt(&i2cCtrl->baseAddrInfo, enable);
}

void I2cStartTransfer(struct I2cCtrl *i2cCtrl)
{
    bool is10BitAddr = false;
    struct I2cMsg *msg = i2cCtrl->transInfo.msg;

    if (msg->flags & I2C_M_TEN) {
        is10BitAddr = true;
    }

    i2cCtrl->ops->TransferInit(&i2cCtrl->baseAddrInfo, msg->addr, is10BitAddr);
    I2cEnableTransInt(i2cCtrl, 1);
}

void I2cTransferMsg(struct I2cCtrl *i2cCtrl)
{
    U8 data;
    U16 isRead;
    bool lastMsg;
    struct I2cMsg *currMsg = NULL;
    bool needStop = false;
    bool needRestart = false;
    int maxWrite = i2cCtrl->ops->GetTxFifoLimit(&i2cCtrl->baseAddrInfo);

    while (i2cCtrl->transInfo.msgTxIdx < i2cCtrl->transInfo.msgNum) {
        currMsg = i2cCtrl->transInfo.msg + i2cCtrl->transInfo.msgTxIdx;
        lastMsg = (i2cCtrl->transInfo.msgTxIdx == i2cCtrl->transInfo.msgNum - 1);
        if (currMsg->buf == NULL) {
            i2cCtrl->transInfo.msgErr = -1;
            break;
        }

        if (i2cCtrl->transInfo.msgTxIdx && !i2cCtrl->transInfo.bufTxIdx) {
            needRestart = true;
        }

        while (i2cCtrl->transInfo.bufTxIdx < currMsg->len && maxWrite > 0) {
            if (i2cCtrl->transInfo.bufTxIdx == currMsg->len - 1 && lastMsg) {
                needStop = true;
            }

            data = currMsg->buf[i2cCtrl->transInfo.bufTxIdx];
            isRead = currMsg->flags & I2C_M_RD;
            i2cCtrl->ops->WriteTxData(&i2cCtrl->baseAddrInfo, data, isRead, needStop, needRestart);
            needRestart = false;
            needStop = false;
            i2cCtrl->transInfo.bufTxIdx++;
            maxWrite--;
        }

        if (i2cCtrl->transInfo.bufTxIdx == currMsg->len) {
            i2cCtrl->transInfo.bufTxIdx = 0;
            i2cCtrl->transInfo.msgTxIdx++;
        }

        if (i2cCtrl->ops->TxFifoFull(&i2cCtrl->baseAddrInfo) == true && maxWrite == 0) {
            break;
        }
    }

    if (i2cCtrl->transInfo.msgTxIdx == i2cCtrl->transInfo.msgNum) {
        i2cCtrl->ops->EnableTxEmptyInt(&i2cCtrl->baseAddrInfo, 0);
    }
}

void I2cReadRxfifo(struct I2cCtrl *i2cCtrl)
{
    struct I2cMsg *currMsg = NULL;

    while(i2cCtrl->transInfo.msgRxIdx < i2cCtrl->transInfo.msgNum) {
        currMsg = i2cCtrl->transInfo.msg + i2cCtrl->transInfo.msgRxIdx;

        if (currMsg->buf == NULL) {
            i2cCtrl->transInfo.msgErr = -1;
            break;
        }

        if (!(currMsg->flags & I2C_M_RD)) {
            i2cCtrl->transInfo.msgRxIdx++;
            continue;
        }

        while (i2cCtrl->ops->RxFifoEmpty(&i2cCtrl->baseAddrInfo) == false && i2cCtrl->transInfo.bufRxIdx < currMsg->len) {
            currMsg->buf[i2cCtrl->transInfo.bufRxIdx++] = i2cCtrl->ops->ReadRxData(&i2cCtrl->baseAddrInfo);
        }

        if (i2cCtrl->transInfo.bufTxIdx == currMsg->len) {
            i2cCtrl->transInfo.bufRxIdx = 0;
            i2cCtrl->transInfo.msgRxIdx++;
        }

        if (i2cCtrl->ops->RxFifoEmpty(&i2cCtrl->baseAddrInfo)) {
            break;
        }
    }

    if (i2cCtrl->transInfo.msgRxIdx == i2cCtrl->transInfo.msgNum) {
        i2cCtrl->ops->EnableRxFullInt(&i2cCtrl->baseAddrInfo, 0);
    }
}

bool WaitForCompltTimeout(struct I2cCtrl *i2cCtrl, U32 timeoutMs)
{
    U64 startCntPct, currCntPct;

    OS_EMBED_ASM("MRS %0, CNTPCT_EL0" : "=r"(startCntPct) : : "memory", "cc");
    do {
        OS_EMBED_ASM("MRS %0, CNTPCT_EL0" : "=r"(currCntPct) : : "memory", "cc");
        if ((currCntPct - startCntPct) / OS_SYS_CLOCK * USEC_PER_MSEC > timeoutMs) {
            return false;
        }
        OS_EMBED_ASM("" : : : "memory");
    } while (i2cCtrl->transInfo.cmdComplete != 1);

    return true;
}

int I2cMasterTransfer(U32 i2cId, struct I2cMsg *msgs, int num)
{
    int ret = 0;
    struct I2cCtrl *i2cCtrl = FindI2cCtrlById(i2cId);
    struct I2cIntStatusInfo intStatus = { 0 };

    i2cCtrl->transInfo.cmdComplete = 0;
    i2cCtrl->ops->GetIntStatusInfo(&i2cCtrl->baseAddrInfo, &intStatus);
    i2cCtrl->ops->ClearAllInt(&i2cCtrl->baseAddrInfo);

    I2cClearTransInfo(i2cCtrl);
    i2cCtrl->transInfo.msgNum = num;
    i2cCtrl->transInfo.msg = msgs;
    I2cStartTransfer(i2cCtrl);

    if (!WaitForCompltTimeout(i2cCtrl, msgs->timeoutMs)) {
        I2cEnableTransInt(i2cCtrl, 0);
        ret = -1;
    }

    if (i2cCtrl->transInfo.msgErr != 0) {
        ret = i2cCtrl->transInfo.msgErr;
    }

    I2cClearTransInfo(i2cCtrl);
    return ret ? ret : num;
}

int I2cIrqHandle(int i2cId)
{
    struct I2cCtrl *i2cCtrl = FindI2cCtrlById(i2cId);
    struct I2cIntStatusInfo intStatus = { 0 };

    i2cCtrl->ops->GetIntStatusInfo(&i2cCtrl->baseAddrInfo, &intStatus);
    if (intStatus.isErrIrq) {
        i2cCtrl->ops->ClearErrInt(&i2cCtrl->baseAddrInfo);
        goto out;
    }

    if (intStatus.isTxEmptyIrq) {
        I2cTransferMsg(i2cCtrl);
        i2cCtrl->ops->ClearTxEmptyInt(&i2cCtrl->baseAddrInfo);
    }

    if (intStatus.isRxFullIrq || intStatus.isTransCpltIrq) {
        I2cReadRxfifo(i2cCtrl);
        i2cCtrl->ops->ClearRxFullInt(&i2cCtrl->baseAddrInfo);
    }

out:
    OS_EMBED_ASM("dmb ish" : : : "memory");
    if (intStatus.isTransCpltIrq || i2cCtrl->transInfo.msgErr != 0) {
        I2cEnableTransInt(i2cCtrl, 0);
        i2cCtrl->ops->ClearTransCpltInt(&i2cCtrl->baseAddrInfo);
        i2cCtrl->transInfo.cmdComplete = 1;
    }

    return 0;
}

int I2cWrite(U32 i2cId, U16 slaveAddr, U16 wrAddr, U8 *txBuf, U32 len)
{
    struct I2cMsg msg;
    U8 sendMsgBuf[len + 2];

    I2C_SET_WRADDR(sendMsgBuf, wrAddr);
    memcpy(sendMsgBuf + 2, txBuf, len);

    msg.addr = slaveAddr;
    msg.flags = 0;
    msg.len = sizeof(sendMsgBuf);
    msg.buf = sendMsgBuf;

    return I2cMasterTransfer(i2cId, &msg, 1);
}

int I2cRead(U32 i2cId, U16 slaveAddr, U16 wrAddr, U8 *rxBuf, U32 len)
{
    U8 offset[2];
    struct I2cMsg msgs[2];

    I2C_SET_WRADDR(offset, wrAddr);
    memset(rxBuf, 0, len);

    msgs[0].addr = slaveAddr;
    msgs[0].flags = 0;
    msgs[0].len = 2;
    msgs[0].buf = offset;

    msgs[1].addr = slaveAddr;
    msgs[1].flags = I2C_M_RD;
    msgs[1].len = len;
    msgs[1].buf = rxBuf;

    return I2cMasterTransfer(i2cId, msgs, 2);
}

void I2cTransferTest(void)
{
    U32 i2cId = 8;
    U16 slaveAddr = 0x50; /* 以eeprom为例 */
    U16 wrAddr = 0x80; /* 从eeprom的0x80位置开始读写 */
    U8 txBuf[20] = "Hello i2c!";
    U8 rxBuf[20] = { 0 };

    (void)I2cWrite(i2cId, slaveAddr, wrAddr, txBuf, sizeof(txBuf));
    PRT_TaskDelay(OS_TICK_PER_SECOND);

    (void)I2cRead(i2cId, slaveAddr, wrAddr, rxBuf, sizeof(rxBuf));
}