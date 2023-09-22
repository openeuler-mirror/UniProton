#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include "posixtest.h"

int gettimeofday_1_1(int argc, char *argv[])
{
    struct timeval tvstandard;
    // Initialize tp
    tvstandard.tv_sec = 0;
    tvstandard.tv_usec = 0;
    if (gettimeofday(&tvstandard, NULL) == 0) {
        if (tvstandard.tv_sec != 0) { // assume this means time was sent
            printf("Test PASSED\n");
            return PTS_PASS;
        } else {
            printf("gettimeofday() success, but tp not filled\n");
            return PTS_FAIL;
        }
    } else {
        printf("gettimeofday() failed\n");
        return PTS_FAIL;
    }

    printf("This code should not be executed.\n");
    return PTS_UNRESOLVED;
}