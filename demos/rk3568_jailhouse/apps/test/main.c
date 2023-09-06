#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "test.h"

#ifdef POSIX_TESTCASE
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#define TEST_TASK_NUM 2
TskHandle g_testTskHandle[TEST_TASK_NUM];
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

void TestTask1()
{
#ifdef POSIX_TESTCASE
    Init(0, 0, 0, 0);
#endif

    while (1) {
        printf("TestTask1 run! \n");
        PRT_TaskDelay(2000);
    }
}

void TestTask2()
{
    while (1) {
        printf("TestTask2 run! \n");
        PRT_TaskDelay(1000);
    }
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // create task1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTask1;
    param.taskPrio = 25;
    param.name = "TestTask1";
    param.stackSize = 0x2000;
    
    ret = PRT_TaskCreate(&g_testTskHandle[0], &param);
    if (ret) {
        return ret;
    }
    
    ret = PRT_TaskResume(g_testTskHandle[0]);
    if (ret) {
        return ret;
    }

    // create task2
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTask2;
    param.taskPrio = 30;
    param.name = "TestTask2";
    param.stackSize = 0x2000;
    
    ret = PRT_TaskCreate(&g_testTskHandle[1], &param);
    if (ret) {
        return ret;
    }
    
    ret = PRT_TaskResume(g_testTskHandle[1]);
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

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = OsHwiInit();
    if (ret) {
        return ret;
    }

    /* 暂不使用uart，先直接写串口寄存器地址 */
    // ret = PRT_PrintfInit();
    // if (ret) {
    //     return ret;
    // }
    
    return OS_OK;
}

void PRT_HardBootInit(void)
{
}

S32 main(void)
{
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
    return ret;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
}

