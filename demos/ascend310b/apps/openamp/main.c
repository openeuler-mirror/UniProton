#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "test.h"
#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shell.h"
#include "show.h"
#endif

TskHandle g_testTskHandle[2];
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];
extern U32 PRT_Printf(const char *format, ...);

#if defined(POSIX_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#if defined(OS_OPTION_OPENAMP)
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
