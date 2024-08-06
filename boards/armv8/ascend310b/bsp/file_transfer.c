#include "file_transfer.h"
#include "ymodem.h"
#include "securec.h"
#include "emmc.h"
#include "pl011.h"
#include "test.h"

static YMODEM_Status PRT_RecvStartCmdCallback(const char *fileName, U32 fileSize, void *userArg)
{
    if (fileName == NULL || fileSize == 0 || userArg == NULL) {
        return YMODEM_STATUS_INVALID_PARAM;
    }
    FILE_DownloadInfo *info = (FILE_DownloadInfo *)userArg;
    if (strcasecmp(info->name, fileName) != 0) {
        return YMODEM_STATUS_ERROR;
    }
    errno_t err = strcpy_s(info->fileName, sizeof(info->fileName), fileName);
    if (err != YMODEM_STATUS_OK) {
        return YMODEM_STATUS_ERROR;
    }
    EMMC_WriteFilePrepare();
    info->curFileLen = 0;
    info->totalFileLen = fileSize;
    return YMODEM_STATUS_OK;
}

static YMODEM_Status PRT_RecvFileDataCallback(U8 seq, const U8 *buf, U32 bufSize, void *userArg)
{
    (void)seq;
    if (buf == NULL || bufSize == 0 || userArg == NULL) {
         return YMODEM_STATUS_INVALID_PARAM;
    }
    FILE_DownloadInfo *info = (FILE_DownloadInfo *)userArg;
    if (info->curFileLen >= info->totalFileLen) {
        return YMODEM_STATUS_INVALID_DATA;
    }
    U32 ret = EMMC_WriteFile((const char *)buf, bufSize);
    if (ret != YMODEM_STATUS_OK) {
        return YMODEM_STATUS_ERROR;
    }
    info->curFileLen += bufSize;
    return YMODEM_STATUS_OK;
}

static YMODEM_Status PRT_RecvEndCmdCallback(void *userArg)
{
    if (userArg == NULL) {
        return YMODEM_STATUS_INVALID_PARAM;
    }
    FILE_DownloadInfo *info = (FILE_DownloadInfo *)userArg;
    if (info->totalFileLen == 0) {
        return YMODEM_STATUS_INVALID_DATA;
    }
    if (EMMC_WriteFileComplete(info->totalFileLen) != YMODEM_STATUS_OK) {
        return YMODEM_STATUS_ERROR;
    }
    return YMODEM_STATUS_OK;
}

U32 PRT_DownloadFile(const char *fileName)
{
    if (fileName == NULL) {
        PRT_Printf("file name is empty\n");
        return YMODEM_STATUS_FILE_NAME_INVALID;
    }
    FILE_DownloadInfo info = {0};
    info.name = fileName;
    YMODEM_ReceiveCallback cb = {
        .recvStartCmd = PRT_RecvStartCmdCallback,
        .recvFileData = PRT_RecvFileDataCallback,
        .recvEndCmd = PRT_RecvEndCmdCallback,
        .userArg = &info,
    };
    YMODEM_Status status = YMODEM_ReceiveFile(&cb);
    if (status == YMODEM_STATUS_OK) {
        PRT_Printf("\nDownload %s successfully: size=%u.\n", info.fileName, info.totalFileLen);
        return YMODEM_STATUS_OK;
    } else {
        PRT_Printf("\nFailed to download %s: err=%d.\n", info.fileName, status);
        return YMODEM_STATUS_ERROR;
    }
}

U32 PRT_GetDownloadFileName(char *fileName, U32 size)
{
    U32 i;
    U32 ret;
    for (i = 0; i < size - 1; i++) {
        unsigned char ch = 0;
        ret = UartGetChar(&ch, YMODEM_READ_TIMEOUT);
        if (ret != YMODEM_STATUS_OK) {
            return ret;
        }
        if ((ch == 0xd) || (ch == 0xa)) {
            fileName[i] = 0;
            return YMODEM_STATUS_OK;
        }
        fileName[i] = ch;
        UartPutChar(ch);
    }
    fileName[i] = 0;
    return YMODEM_STATUS_OK;
}
