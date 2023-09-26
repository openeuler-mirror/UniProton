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
#include "nuttx/serial/serial.h"
#if defined(_SIM_)
#include "semihosting_dbg.h"
#else
#include "rtt_viewer.h"
#endif

#define SHELL_EVENT 0x400

typedef VOID (*pf_OUTPUT)(const CHAR *fmt, ...);

uart_dev_t *stm32_serial_get_uart(int uart_num);
extern void arm_earlyserialinit(void);
extern void arm_serialinit(void);

TskHandle tp1;
TskHandle tp2;

int osShellCmdLs(int argc, const char **argv)
{
    shprintf("*******************shell commands: argc  = %d\n", argc);
    return 0;
}

int test_shell_1()
{
    int ret, fd;
    char *dir;
    uart_dev_t *pdev;
    arm_earlyserialinit();
    arm_serialinit();

    ret = OsShellInit(0);

    ret = osCmdReg(CMD_TYPE_EX, "ls", XARGS, (CMD_CBK_FUNC)osShellCmdLs);
    printf("---- test_shell_1 osCmdReg ret = %d ------\n", ret);

    while(1) {
        PRT_TaskDelay(1000);
    }

    if (fd > 0) {
        close(fd);
    }

    return ret;
}