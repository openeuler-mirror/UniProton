#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "shell.h"

#define INTERVAL_US_DEFAULT 1000
#define LOOP_NUMS_DEFAULT 1000

extern void cyclictest_entry(U64 interval, U64 loopNums);

int OsShellCmdCyclictest(int argc, const char **argv)
{
    if (argc == 0) {
        cyclictest_entry(INTERVAL_US_DEFAULT, LOOP_NUMS_DEFAULT);
        return OS_OK;
    }

    U64 interval, loopNums;
    char *endptr = NULL;
    if (argc == 2) {
        if (!strcmp("-i", argv[0])) {
            interval = (U64)strtoul(argv[1], &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                goto invalid_cmd_input;
            }
            cyclictest_entry(interval, LOOP_NUMS_DEFAULT);
            return OS_OK;
        } else if (!strcmp("-l", argv[0])) {
            loopNums = (U64)strtoul(argv[1], &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                goto invalid_cmd_input;
            }
            cyclictest_entry(INTERVAL_US_DEFAULT, loopNums);
            return OS_OK;
        }
    } else if (argc == 4) {
        if (!strcmp("-i", argv[0]) && !strcmp("-l", argv[2])) {
            interval = (U64)strtoul(argv[1], &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                goto invalid_cmd_input;
            }

            loopNums = (U64)strtoul(argv[3], &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                goto invalid_cmd_input;
            }
            cyclictest_entry(interval, loopNums);
            return OS_OK;
        } else if (!strcmp("-l", argv[0]) && !strcmp("-i", argv[2])) {
            loopNums = (U64)strtoul(argv[1], &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                goto invalid_cmd_input;
            }

            interval = (U64)strtoul(argv[3], &endptr, 0);
            if (endptr == NULL || *endptr != '\0') {
                goto invalid_cmd_input;
            }
            cyclictest_entry(interval, loopNums);
            return OS_OK;
        }
    }

invalid_cmd_input:
    printf("\nUsage: cyclictest [-i] [interval] [-l] [loopNum]\n");
    return OS_OK;
}
