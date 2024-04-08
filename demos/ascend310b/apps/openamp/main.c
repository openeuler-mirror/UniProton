#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "test.h"
#include "prt_timer.h"
#include "prt_queue.h"
#include "rpmsg_backend.h"
#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shell.h"
#include "show.h"
#endif

TskHandle g_testTskHandle[3];
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
U32 g_swtmrId;
extern U32 PRT_Printf(const char *format, ...);

#if defined(POSIX_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#if defined(OS_OPTION_OPENAMP)
extern U32 RpmsgHwiInit(void);

int TestOpenamp()
{
    int ret;
    ret = rpmsg_service_init();
    if (ret) {
        return ret;
    }
    return OS_OK;
}
#endif

#ifdef LOSCFG_SHELL_MICA_INPUT
static int osShellCmdTstReg(int argc, const char **argv)
{
    printf("tstreg: get %d arguments\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("    no %d arguments: %s\n", i + 1, argv[i]);
    }

    return 0;
}

void micaShellInit()
{
    int ret = OsShellInit(0);
    ShellCB *shellCB = OsGetShellCB();
    if (ret != 0 || shellCB == NULL) {
        PRT_Printf("shell init fail\n");
        return;
    }
    (VOID)memset_s(shellCB->shellBuf, SHOW_MAX_LEN, 0, SHOW_MAX_LEN);
    ret = osCmdReg(CMD_TYPE_EX, "tstreg", XARGS, (CMD_CBK_FUNC)osShellCmdTstReg);
    if (ret == 0) {
        PRT_Printf("[INFO]: reg cmd 'tstreg' successed!\n");
    } else {
        PRT_Printf("[INFO]: reg cmd 'tstreg' failed!\n");
    }
}
#endif

static void QueueReadTest()
{
    U32 ret, queueId;
    ret = PRT_QueueCreate(2, 16, &queueId);
    if (ret != OS_OK) {
        PRT_Printf("ReadTest, PRT_QueueCreate fail.\n");
        return;
    }

    char buf[16];
    int len = sizeof(buf);
    ret = PRT_QueueRead(queueId, buf, &len, OS_QUEUE_WAIT_FOREVER);
    if (ret != OS_OK) {
        PRT_Printf("ReadTest, PRT_QueueRead fail.\n");
        return;
    }

    return;
}

static void QueueWriteTest()
{
    U32 ret, queueId;
    ret = PRT_QueueCreate(2, 16, &queueId);
    if (ret != OS_OK) {
        PRT_Printf("WriteTest, PRT_QueueCreate fail.\n");
        return;
    }

    char buf[16] = {1};
    ret = PRT_QueueWrite(queueId, buf, 16, OS_QUEUE_WAIT_FOREVER, OS_QUEUE_NORMAL);

    if (ret != OS_OK) {
        PRT_Printf("WriteTest, PRT_QueueWrite fail.\n");
        return;
    }

    return;
}

void Test2TaskEntry()
{
    QueueReadTest();
    while (1) {
        PRT_Printf("Test2TaskEntry run!!! \n");
        PRT_TaskDelay(2000);
    }
}

void Test3TaskEntry()
{
    QueueWriteTest();
    while (1) {
        PRT_Printf("Test3TaskEntry run!!! \n");
        PRT_TaskDelay(2000);
    }
}

U32 QueueTest()
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // task 2
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)Test2TaskEntry;
    param.taskPrio = 30;
    param.name = "Test2Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle[1], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle[1]);
    if (ret) {
        return ret;
    }

    // task 3
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)Test3TaskEntry;
    param.taskPrio = 25;
    param.name = "Test3Task";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle[2], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle[2]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

void Test1TaskEntry()
{
#if defined(OS_OPTION_OPENAMP)
    TestOpenamp();
#endif

#ifdef LOSCFG_SHELL_MICA_INPUT
    micaShellInit();
#endif

#if defined(POSIX_TESTCASE)
    Init(0, 0, 0, 0);
#endif

#if defined(OS_OPTION_QUEUE)
    QueueTest();
#endif
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // task 1
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)Test1TaskEntry;
    param.taskPrio = 25;
    param.name = "Test1Task";
    param.stackSize = 0x2000;
    
    ret = PRT_TaskCreate(&g_testTskHandle[0], &param);
    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle[0]);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

static void TimerTestCallback(TimerHandle tmrHandle, U32 arg1, U32 arg2, U32 arg3, U32 arg4)
{
    return;
}

U32 TimerTestStart()
{
    U32 ret;
    struct TimerCreatePara timer = {0};

    timer.type = OS_TIMER_SOFTWARE;
    timer.mode = OS_TIMER_LOOP;
    timer.interval = 1000;
    timer.timerGroupId = 0;
    timer.callBackFunc = TimerTestCallback;

    ret = PRT_TimerCreate(&timer, &g_swtmrId);
    if (ret != OS_OK) {
        return OS_ERROR;
    }

    ret = PRT_TimerStart(0, g_swtmrId);
    if (ret != OS_OK) {
        (void)PRT_TimerDelete(0, g_swtmrId);
        return OS_ERROR;
    }

    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;

#if defined(OS_OPTION_OPENAMP)
    /*
     * Linux will send an interrupt to Uniproton after initialising vdev.
     * However, if Uniproton has not registered the corresponding IPI handler,
     * it will throw an exception (call OsHwiDefaultHandler()).
     *
     * Therefore, even though we may not need to handle the interrupts sent by Linux
     * until the rpmsg backend is initialised, we also need to register the IPI handler
     * before irq_enable.
     * We will register the actual interrupt handler when rpmsg is initialised.
     */
    ret = RpmsgHwiInit();
    if (ret) {
        return ret;
    }
#endif

    ret = OsTestInit();
    if (ret) {
        return ret;
    }

    ret = TestClkStart();
    if (ret) {
        return ret;
    }

    ret = TimerTestStart();
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
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}
