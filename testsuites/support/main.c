#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#if defined(_SIM_)
#include "semihosting_dbg.h"
#else
#include "rtt_viewer.h"
#endif
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);

/* From newlib init.c */
extern void (*__preinit_array_start []) (void) __attribute__((weak));
extern void (*__preinit_array_end []) (void) __attribute__((weak));
extern void (*__init_array_start []) (void) __attribute__((weak));
extern void (*__init_array_end []) (void) __attribute__((weak));

extern void _init (void);

/* Iterate over all the init routines.  */
void __libc_init_array (void)
{
    size_t count;
    size_t i;

    count = __preinit_array_end - __preinit_array_start;
    for (i = 0; i < count; i++)
        __preinit_array_start[i] ();

    _init ();

    count = __init_array_end - __init_array_start;
    for (i = 0; i < count; i++)
        __init_array_start[i] ();
}

// 尚未适配，暂时先实现
int puts(const char *s)
{
    return printf("%s\n", s);
}
void perror(const char *s)
{
    printf("%s\n", s);
}

U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle taskPid;
    struct TskInitParam initParam = {Init, OS_TSK_PRIORITY_10, 0, { 0 }, 0x2000, "MainTask", 0};
    ret = PRT_TaskCreate(&taskPid, &initParam);

    if (ret) {
        return ret;
    }

    ret = PRT_TaskResume(taskPid);
    if (ret) {
        return ret;
    }

    return OS_OK;
}

U32 PRT_HardDrvInit(void)
{
#if !defined(_SIM_)
    RttViewerInit();
    RttViewerModeSet(0, RTT_VIEWER_MODE_BLOCK_IF_FIFO_FULL);
#endif

    return OS_OK;
}

U32 g_testRandStackProtect;
void OsRandomSeedInit(void)
{
#if defined(OS_OPTION_RND)
    U32 ret;
    U32 seed;
    seed = PRT_ClkGetCycleCount64();
    g_testRandStackProtect = rand_r(&seed);
    ret = PRT_SysSetRndNum(OS_SYS_RND_STACK_PROTECT, g_testRandStackProtect);
#endif
}

extern U32 __data_start__;
extern U32 __data_end__;
extern U32 __text_end__;

void OsGlobalDataInit()
{
    U32 size;
    U32 *dest = (U32 *)&__data_start__;
    U32 *src = (U32 *)&__text_end__;
    U32 i;

    size = (U32)&__data_end__ - (U32)&__data_start__;
    for (i = 0; i < (size / 4); i++) {
        dest[i] = src[i];
    }
}

void PRT_HardBootInit()
{
    OsGlobalDataInit();
    OsRandomSeedInit();
}

// U32 PRT_Printf(const char *format, ...)
int printf(const char *format, ...)
{
    va_list vaList;
    char buff[0x200] = { 0 };
    S32 count;
    U32 ret;

    va_start(vaList, format);
    count = vsprintf_s(buff, 0x200, format, vaList);
    va_end(vaList);

    if (count == -1) {
        return OS_ERROR;
    }

#if defined(_SIM_)
    SemihostingDbgWrite(buff, count);
#else
    RttViewerWrite(0, buff, count);
#if defined(PRINT_USE_UART)
    int fd = open(SHELL_PATH, O_RDWR | O_NOCTTY | O_NDELAY);
    write(fd, buff, count);
    close(fd);
#endif
#endif

    return count;
}

S32 main(void)
{
    return OsConfigStart();
}
