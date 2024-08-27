#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <fcntl.h>
#include <termios.h>
#include "prt_task.h"
#include "shell.h"
#include "console.h"
#include "show.h"
#include "nuttx/serial/serial.h"
#include "test.h"
#include "rtt_viewer.h"
#include "securec.h"
#include "prt_config_internal.h"
#include "nuttx/sys/sys_fcntl.h"
#include "nuttx/sys/sys_unistd.h"

#define SHELL_EVENT 0x400

extern void arm_earlyserialinit(void);
extern void arm_serialinit(void);

int osShellCmdTstReg(int argc, const char **argv)
{
    shprintf("tstreg: get %d arguments\n", argc);
    for(int i = 0; i < argc; i++) {
        shprintf("    no %d arguments: %s\n", i + 1, argv[i]);
    }

    return 0;
}

void shell_task(U32 uwParam1, U32 uParam2, U32 uwParam3, U32 uwParam4)
{
    int ret;
    char *dir;
    uart_dev_t *pdev;
    arm_earlyserialinit();
    arm_serialinit();

    ret = OsShellInit(0);

    ret = osCmdReg(CMD_TYPE_EX, "tstreg", XARGS, (CMD_CBK_FUNC)osShellCmdTstReg);
    if (ret == 0) {
        printf("[INFO]: reg cmd 'tstreg' successed!\n");
    } else {
        printf("[INFO]: reg cmd 'tstreg' failed!\n");
    }
    
    while (1) {
        PRT_TaskDelay(1000);
    }
}

U32 PRT_AppInit(void)
{
    U32 ret;
    TskHandle taskPid;
    struct TskInitParam stInitParam = {shell_task, 10, 0, {0}, 0x800, "TaskA", 0};

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
    int fd;

    va_start(vaList, format);
    count = vsprintf_s(buff, 0x200, format, vaList);
    va_end(vaList);

    if (count == -1) {
        return OS_ERROR;
    }

    RttViewerWrite(0, buff, count);
    fd = sys_open(SHELL_PATH, O_RDWR | O_NOCTTY | O_NDELAY);
    sys_write(fd, buff, count);
    sys_close(fd);

    return count;
}

S32 main(void)
{
    return OsConfigStart();
}
