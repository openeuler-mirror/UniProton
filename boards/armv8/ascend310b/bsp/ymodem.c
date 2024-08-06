#include "ymodem.h"
#include <limits.h>
#include "securec.h"
#include "prt_mem.h"
#include "test.h"
#include "pl011.h"

static bool g_enable = false;

bool YMODEM_IsEnable(void)
{
    return g_enable;
}

void YMODEM_Enable(void)
{
    g_enable = true;
}

void YMODEM_Disable(void)
{
    g_enable = false;
}

static void YMODEM_SendNak(void)
{
    UartPutChar(YMODEM_NAK);
}

static void YMODEM_SendAck(void)
{
    UartPutChar(YMODEM_ACK);
}

static void YMODEM_SendAbortCmd(void)
{
    UartPutChar(YMODEM_CAN);
    UartPutChar(YMODEM_CAN);
}

static void YMODEM_SendRequestCmd(void)
{
    UartPutChar(YMODEM_C);
}

static U16 YMODEM_CalculateCrc16Ymodem(const U8 *data, U32 dataSize)
{
    U16 crc = CRC_16_YMODEM_INIT_VALUE;
    for (U32 i = 0; i < dataSize; i++) {
        crc ^= (U16)((U16)data[i] << CHAR_BIT);
        for (U8 j = 0; j < CHAR_BIT; j++) {
            if (crc & CRC_16_MOST_SIGNIFICANT_BIT) {
                crc = (U16)(crc << 1) ^ CRC_16_YMODEM_POLY;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

static YMODEM_Status YMODEM_RecvPacket(YMODEM_Packet *packet)
{
    U32 dataSize = 0;
    if (UartGetChar(&packet->type, YMODEM_READ_TIMEOUT) != 0) {
        return YMODEM_STATUS_TIMEOUT;
    }
    switch (packet->type) {
        case YMODEM_SOH:
            dataSize = sizeof(packet->soh.data);
            break;
        case YMODEM_STX:
            dataSize = sizeof(packet->stx.data);
            break;
        case YMODEM_EOT:
            return YMODEM_STATUS_OK;
        case YMODEM_CAN:
            if (UartGetChar(&packet->type, YMODEM_READ_TIMEOUT) != 0) {
                return YMODEM_STATUS_TIMEOUT;
            }
            if (packet->type == YMODEM_CAN) {
                return YMODEM_STATUS_ABORT_BY_SENDER;
            } else {
                return YMODEM_STATUS_UNKNOWN_CMD;
            }
        case YMODEM_ABORT1:
        case YMODEM_ABORT2:
            return YMODEM_STATUS_ABORT_BY_USER;
        default:
            return YMODEM_STATUS_UNKNOWN_CMD;
    }
    U8 *buf = (U8 *)packet;
    for (U32 i = sizeof(packet->type); i < dataSize + YMODEM_PACKET_OVERHEAD; i++) {
        if (UartGetChar(&buf[i], YMODEM_READ_TIMEOUT) != 0) {
            return YMODEM_STATUS_TIMEOUT;
        }
    }
    return YMODEM_STATUS_OK;
}

static YMODEM_Status YMODEM_ParseControlFrame(const U8 *data, const YMODEM_ReceiveCallback *cb,
    YMODEM_Session *session)
{
    /* 结束帧，数据全0。会话结束，回复ack后正常退出 */
    if (data[0] == 0) {
        session->isFileEnd = true;
        session->state = YMODEM_SESSION_STATE_DONE;
        YMODEM_SendAck();
        return YMODEM_STATUS_OK;
    }
    /* 起始帧，数据为文件名和文件大小 */
    char fileSizeStr[YMODEM_FILE_SIZE_MAX_LEN + 1] = {0};
    const U8 *filePtr = data;
    for (U32 i = 0; i < YMODEM_FILE_NAME_MAX_LEN && *filePtr != 0; i++, filePtr++) {
        session->fileName[i] = *filePtr;
    }
    filePtr++;
    for (U32 i = 0; i < YMODEM_FILE_SIZE_MAX_LEN && *filePtr != ' '; i++, filePtr++) {
        fileSizeStr[i] = *filePtr;
    }
    S32 ret = sscanf_s(fileSizeStr, "%lu", &session->totalFileSize);
    if (ret != 1) {
        return YMODEM_STATUS_INVALID_DATA;
    }
    if (cb->recvStartCmd((char *)session->fileName, session->totalFileSize, cb->userArg) != YMODEM_STATUS_OK) {
        return YMODEM_STATUS_CALLBACK_ERR;
    }
    session->state = YMODEM_SESSION_STATE_BUSY;
    session->seq++;
    YMODEM_SendAck();
    YMODEM_SendRequestCmd();
    return YMODEM_STATUS_OK;
}

static YMODEM_Status YMODEM_ParseDataFrame(const U8 *data, U32 dataSize, const YMODEM_ReceiveCallback *cb,
    YMODEM_Session *session)
{
    if (session->curFileSize >= session->totalFileSize) {
        return YMODEM_STATUS_INVALID_DATA;
    } else if (session->curFileSize + dataSize > session->totalFileSize) {
        dataSize = session->totalFileSize - session->curFileSize;
    }
    if (cb->recvFileData(session->seq, data, dataSize, cb->userArg) != YMODEM_STATUS_OK) {
        return YMODEM_STATUS_CALLBACK_ERR;
    }
    session->curFileSize += dataSize;
    session->errCnt = 0;
    session->seq++;
    YMODEM_SendAck();
    return YMODEM_STATUS_OK;
}

static YMODEM_Status YMODEM_ParseRecvPacket(const YMODEM_Packet *packet, const YMODEM_ReceiveCallback *cb,
    YMODEM_Session *session)
{
    /* 文件传输结束，回复ACK和请求下帧命令后，结束该次传输并继续接收命令 */
    if (packet->type == YMODEM_EOT) {
        if (cb->recvEndCmd(cb->userArg) != YMODEM_STATUS_OK) {
            return YMODEM_STATUS_CALLBACK_ERR;
        }
        session->isFileEnd = true;
        session->state = YMODEM_SESSION_STATE_IDLE;
        YMODEM_SendAck();
        YMODEM_SendRequestCmd();
        return YMODEM_STATUS_OK;
    }
    const U8 *data = (packet->type == YMODEM_SOH) ? packet->soh.data : packet->stx.data;
    U32 dataSize = (packet->type == YMODEM_SOH) ? sizeof(packet->soh.data) : sizeof(packet->stx.data);
    U16 crc = (U16)((packet->type == YMODEM_SOH) ? YMODEM_NTOHS(packet->soh.crc) : YMODEM_NTOHS(packet->stx.crc));
    U8 seq = (packet->seqOc ^ 0xFF);
    if (packet->seq != seq) {
        return YMODEM_STATUS_SEQ_INVALID;
    }
    if (YMODEM_CalculateCrc16Ymodem(data, dataSize) != crc) {
        return YMODEM_STATUS_CRC_INVALID;
    }
    if (packet->seq != session->seq) {
        return YMODEM_STATUS_SEQ_MISMATCH;
    }
    if (session->seq == 0 && session->state == YMODEM_SESSION_STATE_IDLE) {
        return YMODEM_ParseControlFrame(data, cb, session);
    } else {
        return YMODEM_ParseDataFrame(data, dataSize, cb, session);
    }
}

/*
* 主动中止：发送中止命令后退出
* 发送端中止：回复ACK后退出
* 数据包序列号不匹配：回复NAK后继续接收命令
* 其他错误：超过最大错误次数，发送中止命令后退出，否则发送请求命令
*/
static void YMODEM_HandleErrorStatus(YMODEM_Status status, YMODEM_Session *session)
{
    switch (status) {
        case YMODEM_STATUS_OK:
            break;
        case YMODEM_STATUS_SEQ_MISMATCH:
            YMODEM_SendNak();
            break;
        case YMODEM_STATUS_ABORT_BY_SENDER:
            session->isFileEnd = true;
            session->state = YMODEM_SESSION_STATE_DONE;
            YMODEM_SendAck();
            break;
        case YMODEM_STATUS_CALLBACK_ERR:
        case YMODEM_STATUS_ABORT_BY_USER:
            session->isFileEnd = true;
            session->state = YMODEM_SESSION_STATE_DONE;
            YMODEM_SendAbortCmd();
            break;
        default:
            if (session->state == YMODEM_SESSION_STATE_BUSY) {
                session->errCnt++;
            }
            if (session->errCnt > YMODEM_MAX_ERRORS) {
                session->isFileEnd = true;
                session->state = YMODEM_SESSION_STATE_DONE;
                YMODEM_SendAbortCmd();
            } else {
                YMODEM_SendRequestCmd();
            }
            break;
    }
}

YMODEM_Status YMODEM_ReceiveFile(const YMODEM_ReceiveCallback *cb)
{
    if (cb == NULL || cb->recvStartCmd == NULL || cb->recvFileData == NULL || cb->recvEndCmd == NULL) {
        return YMODEM_STATUS_INVALID_PARAM;
    }
    YMODEM_Packet *packet = PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, sizeof(YMODEM_Packet), MEM_ADDR_ALIGN_016);
    if (packet == NULL) {
        return YMODEM_STATUS_OUT_OF_MEMORY;
    }
    (void)memset_s(packet, sizeof(YMODEM_Packet), 0, sizeof(YMODEM_Packet));
    YMODEM_Enable();
    YMODEM_Session session;
    YMODEM_Status status;
    do {
        (void)memset_s(&session, sizeof(session), 0, sizeof(session));
        do {
            status = YMODEM_RecvPacket(packet);
            if (status == YMODEM_STATUS_OK) {
                status = YMODEM_ParseRecvPacket(packet, cb, &session);
            }
            YMODEM_HandleErrorStatus(status, &session);
        } while(!session.isFileEnd);
    } while(session.state != YMODEM_SESSION_STATE_DONE);
    (void)PRT_MemFree(0, packet);
    YMODEM_Disable();
    return status;
}
