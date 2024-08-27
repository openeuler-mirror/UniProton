#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "test.h"
#include "timer.h"
#include "hwi_init.h"
#include "prt_config.h"
#include "uniproton_shm_demo.h"
#include "spi_1911.h"
#include "uniproton_its_demo.h"
#include "pl011.h"
#include "file_transfer.h"
#include "ymodem.h"
#include "i2c_1911.h"

#define FILE_NAME_LEN 32
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
TskHandle g_testTskHandle;

void TestTaskEntry(void)
{
    U32 ret;
    PRT_Printf("\nUniProton #");
    while (1) {
        unsigned char ch = 0;
        UartGetChar(&ch, YMODEM_READ_TIMEOUT);
        if (ch == 0xd) {
            PRT_Printf("\nUniProton #");
        } else if (ch == 0x2) {
            PRT_Printf("\nDownload file name:");
            char fileName[FILE_NAME_LEN] = {0};
            ret = PRT_GetDownloadFileName(fileName, FILE_NAME_LEN);
            if (ret != 0) {
                PRT_Printf("\nUniProton #");
                continue;
            }
            PRT_Printf("\nStart downloading...");
            PRT_DownloadFile(fileName);
            PRT_Printf("\nUniProton #");
        } else {
            UartPutChar(ch);
        }
    }
    return;
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    param.stackAddr = (uintptr_t)PRT_MemAllocAlign(0, ptNo, 0x8000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = 25;
    param.name = "TestTask";
    param.stackSize = 0x8000;

    ret = PRT_TaskCreate(&g_testTskHandle, &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;
    ret = OsTestInit();
    if (ret) {
        return ret;
    }

    ret = TestClkStart();
    if (ret) {
        return ret;
    }

    ret = TestShmStart();
    if (ret) {
        return ret;
    }

    ret = PRT_UartInterruptInit();
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = OsHwiInit();
    if (ret) {
        return ret;
    }

#if (CONFIG_SPI_ENABLE == YES)
    ret = OsSpiInit();
    if (ret) {
        return ret;
    }
#endif

#if (CONFIG_I2C_ENABLE == YES)
    ret = I2cInit();
    if (ret) {
        return ret;
    }
#endif

    return OS_OK;
}

void PRT_HardBootInit(void)
{
}

S32 main(void)
{
    PRT_UartInit();
    U64 t;
    asm volatile("mrs %0, CNTPCT_EL0\n" : "=r" (t));
    PRT_Printf("[uniproton] start time 0.%llu s\n", t / 48);
    return OsConfigStart();
}

extern void *__wrap_memset(void *dest, int set, U32 len)
{
    if (dest == NULL || len == 0) {
        return NULL;
    }

    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i) {
        ret[i] = set;
    }
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}