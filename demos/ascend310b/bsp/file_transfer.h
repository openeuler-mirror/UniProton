#ifndef FILE_TRANSFER_H
#define FILE_TRANSFER_H

#include "prt_typedef.h"

#define FILE_NAME_LEN_MAX 64
typedef struct {
    U32        curFileLen;
    U32        totalFileLen;
    char       fileName[FILE_NAME_LEN_MAX];
    const char *name;
} FILE_DownloadInfo;
U32 PRT_DownloadFile(const char *fileName);
U32 PRT_GetDownloadFileName(char *fileName, U32 size);
#endif
