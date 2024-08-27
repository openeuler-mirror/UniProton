#ifndef YMODEM_H
#define YMODEM_H

#include "prt_typedef.h"

#define CRC_16_MOST_SIGNIFICANT_BIT      0x8000U
#define CRC_16_YMODEM_POLY               0x1021U
#define CRC_16_YMODEM_INIT_VALUE         0x0000U
#define YMODEM_PACKET_OVERHEAD           5        // 除了数据载荷之外的其他字段开销
#define YMODEM_DATA_SIZE_128B            128      // SOH 128字节数据大小
#define YMODEM_DATA_SIZE_1024B           1024     // STX 1024字节数据大小
#define YMODEM_FILE_NAME_MAX_LEN         64       // 文件名最大长度，不包含结束符
#define YMODEM_FILE_SIZE_MAX_LEN         9        // 文件大小最大长度，不包含结束符
#define YMODEM_READ_TIMEOUT              1000     // 读超时 ms
#define YMODEM_MAX_ERRORS                5        // 传输出错的最大次数

/* 小端机器上把16位整数的2字节倒转，例如0x1234变成0x3412 */
#define YMODEM_NTOHS(x) ((((x) & 0x00ff) << 8) | (((x) & 0xff00) >> 8))

typedef enum {
    YMODEM_STATUS_OK = 0,
    YMODEM_STATUS_ERROR,
    YMODEM_STATUS_OUT_OF_MEMORY,
    YMODEM_STATUS_INVALID_PARAM,
    YMODEM_STATUS_TIMEOUT,
    YMODEM_STATUS_ABORT_BY_SENDER,
    YMODEM_STATUS_ABORT_BY_USER,
    YMODEM_STATUS_UNKNOWN_CMD,
    YMODEM_STATUS_INVALID_DATA,
    YMODEM_STATUS_SEQ_MISMATCH,
    YMODEM_STATUS_SEQ_INVALID,
    YMODEM_STATUS_CRC_INVALID,
    YMODEM_STATUS_CALLBACK_ERR,
    YMODEM_STATUS_FILE_NAME_INVALID,
    YMODEM_STATUS_MAX
} YMODEM_Status;

typedef struct {
    YMODEM_Status (*recvStartCmd)(const char *fileName, U32 fileSize, void *userArg);
    YMODEM_Status (*recvFileData)(U8 seq, const U8 *buf, U32 bufSize, void *userArg);
    YMODEM_Status (*recvEndCmd)(void *userArg);
    void *userArg;
} YMODEM_ReceiveCallback;

typedef enum {
    YMODEM_SOH    = 0x01U, /* start of 128-byte data packet */
    YMODEM_STX    = 0x02U, /* start of 1024-byte data packet */
    YMODEM_EOT    = 0x04U, /* end of transmission */
    YMODEM_ACK    = 0x06U, /* acknowledge */
    YMODEM_NAK    = 0x15U, /* negative acknowledge */
    YMODEM_CAN    = 0x18U, /* two consecutive CAN characters as a transfer abort cmd */
    YMODEM_C      = 0x43U, /* request the frist block with 16 bits crc */
    YMODEM_ABORT1 = 0x41U, /* abort by user 'A' */
    YMODEM_ABORT2 = 0x61U, /* abort by user 'a' */
} YMODEM_Type;

#pragma pack(1)
typedef struct {
    U8 type;
    U8 seq;
    U8 seqOc;
    union {
        struct {
            U8 data[YMODEM_DATA_SIZE_128B];
            U16 crc;
        } soh;
        struct {
            U8 data[YMODEM_DATA_SIZE_1024B];
            U16 crc;
        } stx;
    };
} YMODEM_Packet;
#pragma pack()

typedef enum {
    YMODEM_SESSION_STATE_IDLE,
    YMODEM_SESSION_STATE_BUSY,
    YMODEM_SESSION_STATE_DONE,
    YMODEM_SESSION_STATE_MAX
} YMODEM_SessionState;

typedef struct {
    U8        seq;
    U8        errCnt;
    bool      isFileEnd;
    YMODEM_SessionState state;
    char      fileName[YMODEM_FILE_NAME_MAX_LEN + 1];
    U32       totalFileSize;
    U32       curFileSize;
} YMODEM_Session;

bool YMODEM_IsEnable(void);
void YMODEM_Enable(void);
void YMODEM_Disable(void);
YMODEM_Status YMODEM_ReceiveFile(const YMODEM_ReceiveCallback *cb);

#endif
