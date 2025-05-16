#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"
#include "prt_log.h"
#include "prt_task.h"

#define LOG_LEVEL_NUM 8

static const char *g_logSwitchStr[2] = {"OFF", "ON"};
static const char *g_logLevelStr[LOG_LEVEL_NUM + 1] = {
    "EMERG", "ALERT", "CRIT", "ERR", "WARN",
    "NOTICE", "INFO", "DEBUG", "NONE"
};

static void ShellSetLogLevel(const char *levelStr)
{
    U32 ret, levelNum;
    char *endptr = NULL;
    levelNum = (U32)strtoul(levelStr, &endptr, 0);
    if (endptr == NULL || *endptr != '\0') {
        printf("Log level str %s is not defined!\n", levelStr);
        return;
    }

    if (levelNum < (U32)OS_LOG_ALERT || levelNum > (U32)OS_LOG_NONE) {
        printf("Log level %d is out of range!\n", levelNum);
        return;
    }

    ret = PRT_LogSetFilter((enum OsLogLevel)levelNum);
    if (ret != OS_OK) {
        printf("Set Log level %d failed, ret is %d\n", levelNum, ret);
    } else {
        printf("Set Log level %d success!\n", levelNum);
    }
}

int OsShellCmdLog(int argc, const char **argv)
{
    if (!PRT_IsLogInit()) {
        printf("Log module init failed!\n");
        return OS_OK;
    }

    if (argc == 1) {
        if (!strcmp("on", argv[0])) {
            PRT_LogOn();
            printf("---Log ON---\n");
            return OS_OK;
        } else if (!strcmp("off", argv[0])) {
            PRT_LogOff();
            printf("---Log OFF---\n");
            return OS_OK;
        } else if (!strcmp("status", argv[0])) {
            U8 switchStat, levelStat;
            PRT_LogGetStatus(&switchStat, &levelStat);
            printf("Log switch: %s, Log level: %s\n",
                g_logSwitchStr[switchStat], g_logLevelStr[levelStat]);
            return OS_OK;
        }
    } else if (argc == 2) {
        if (!strcmp("set", argv[0])) {
            ShellSetLogLevel(argv[1]);
            return OS_OK;
        }
    }

invalid_cmd_input:
    printf("\nUsage: log on/off/status/set [level]\n");
    return OS_OK;
}
