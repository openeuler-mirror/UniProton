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

U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
TskHandle g_testTskHandle;

void TestTaskEntry()
{
    U64 n = 0;

    while (++n) {
        PRT_TaskDelay(OS_TICK_PER_SECOND);
        PRT_Printf("[uniproton] test [%llu]\n", n);

        SpiTransferTest();
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