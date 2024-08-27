#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "securec.h"
#include "prt_config.h"
#include "prt_config_internal.h"
#include "prt_clk.h"
#include "prt_idle.h"
#if defined(_SIM_)
#include "semihosting_dbg.h"
#else
#include "rtt_viewer.h"
#endif

extern void arm_earlyserialinit(void);
extern void arm_serialinit(void);
extern int group_setupidlefiles();

void stdio_task(U32 uwParam1, U32 uParam2, U32 uwParam3, U32 uwParam4)
{
    arm_earlyserialinit();
    arm_serialinit();
    (void)group_setupidlefiles();

    int val;
    char str[32] = {0};
    setbuf(stdout, NULL);
    while (1) {
        printf("Please input a string(<32): \n");
        scanf("%s", str);
        printf("The string is %s.\n", str);
        PRT_TaskDelay(100);
    }

    return;
}

U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle taskPid;
    struct TskInitParam stInitParam = {stdio_task, 10, 0, {0}, 0x800, "TaskA", 0};

    ret = PRT_TaskCreate(&taskPid, &stInitParam);
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

extern U32 __data_start__;
extern U32 __data_end__;
extern U32 __text_end__;
void OsGlobalDataInit(void)
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

void PRT_HardBootInit(void)
{
    OsGlobalDataInit();
}

U32 PRT_Printf(const char *format, ...)
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
#endif

    return count;
}

S32 main(void)
{
    return OsConfigStart();
}
