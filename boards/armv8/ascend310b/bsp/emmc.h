#ifndef EMMC_H
#define EMMC_H

#include "prt_typedef.h"

void EMMC_WriteFilePrepare(void);
U32 EMMC_WriteFile(const char *data, U32 size);
U32 EMMC_WriteFileComplete(U32 size);

#endif
