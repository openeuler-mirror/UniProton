#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_task.h"
#include "prt_clk.h"
#include "prt_hwi.h"
#include "prt_sys.h"
#include "prt_tick.h"
#include "cpu_config.h"
#include "test.h"
#include <mica/mica.h>
#include <mica/service.h>
#include <mica/debug.h>
#ifdef LOSCFG_SHELL_MICA_INPUT
#include "shell.h"
#include "show.h"
#include "shmsg.h"
#endif

TskHandle g_testTskHandle;
U8 g_memRegion00[OS_MEM_FSC_PT_SIZE];

#ifdef LOSCFG_SHELL_MICA_INPUT
static int osShellCmdTstReg(int argc, const char **argv)
{
    printf("tstreg: get %d arguments\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("    no %d arguments: %s\n", i + 1, argv[i]);
    }

    return 0;
}

static int micaShellInit(void)
{
    int ret = OsShellInit(0);
    ShellCB *shellCB = OsGetShellCB();
    if (ret != 0 || shellCB == NULL) {
        PRT_Printf("shell init fail\n");
        return -1;
    }
    (VOID)memset_s(shellCB->shellBuf, SHOW_MAX_LEN, 0, SHOW_MAX_LEN);
    ret = osCmdReg(CMD_TYPE_EX, "tstreg", XARGS, (CMD_CBK_FUNC)osShellCmdTstReg);
    if (ret == 0) {
        PRT_Printf("[INFO]: reg cmd 'tstreg' successed!\n");
    } else {
        PRT_Printf("[INFO]: reg cmd 'tstreg' failed!\n");
    }
    return ret;
}
static void micaShellChar(char c)
{
    ShellCmdLineParse(c, (pf_OUTPUT)printf, OsGetShellCB());
}
#endif

void TestTaskEntry()
{
    U32 a = 5;

    /* The optional product debug producer was explicitly initialized after
     * OS memory/HWI setup, before domain facilities. Never redirect printf. */
    (void)mica_debug_printf("UniProton Xen TestTask ready\n");
    PRT_Printf("Testing UniProton timer...\n");
    while (a--)
    {
        PRT_ClkDelayMs(500);
        PRT_Printf("tick=%d\n", PRT_TickGetCount());
    }

#ifdef LOSCFG_SHELL_MICA_INPUT
    if (micaShellInit()) return;
#endif
    struct mica_config config={0};
#ifdef LOSCFG_SHELL_MICA_INPUT
    config.sys_ops.shell_cmd_handler=micaShellChar;
#endif
#ifdef OS_OPTION_POWEROFF
    config.sys_ops.system_poweroff=PRT_SysPowerOff;
#endif
    int ret=mica_init(&config);
    if (!ret) ret=mica_create_service(MICA_SERVICE_TTY);
    if (ret) {
        (void)mica_debug_printf("MICA TTY startup failed: %d\n", ret);
        PRT_Printf("MICA TTY startup failed: %d\n",ret);
        if (!mica_stop_services()) mica_remove();
        return;
    }
    PRT_Printf("MICA TTY locally started (peer binding may follow)\n");

    return;
}

U32 OsTestInit(void)
{
    U32 ret;
    U8 ptNo = OS_MEM_DEFAULT_FSC_PT;
    struct TskInitParam param = {0};

    // create task
    param.stackAddr = PRT_MemAllocAlign(0, ptNo, 0x2000, MEM_ADDR_ALIGN_016);
    param.taskEntry = (TskEntryFunc)TestTaskEntry;
    param.taskPrio = 30;
    param.name = "TestTask";
    param.stackSize = 0x2000;

    ret = PRT_TaskCreate(&g_testTskHandle, &param);
    if (ret)
    {
        return ret;
    }

    ret = PRT_TaskResume(g_testTskHandle);
    if (ret)
    {
        return ret;
    }

    return OS_OK;
}

U32 PRT_AppInit(void)
{
    U32 ret;

    ret = OsTestInit();
    if (ret)
    {
        return ret;
    }
    ret = TestClkStart();
    if (ret)
    {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
    U32 ret;

    ret = OsHwiInit();
    if (ret)
    {
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
    if (dest == NULL || len == 0)
    {
        return NULL;
    }

    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i)
    {
        ret[i] = set;
    }
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i)
    {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}
