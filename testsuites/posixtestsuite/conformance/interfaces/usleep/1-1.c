#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include "posixtest.h"

int usleep_1_1(int argc, char *argv[])
{
    struct timespec tssleepfor, tsbefore, tsafter;
    int sleepnsec = 3000;
    long slepts = 0,sleptns = 0;

    if (clock_gettime(CLOCK_REALTIME, &tsbefore) == -1) {
        perror("Error in clock_gettime()\n");
        return PTS_UNRESOLVED;
    }

    tssleepfor.tv_sec = 0;
    tssleepfor.tv_nsec = sleepnsec;
    if (usleep((unsigned int)sleepnsec / 1000) != 0) {
        printf("usleep() did not return success\n");
        return PTS_UNRESOLVED;
    }

    if (clock_gettime(CLOCK_REALTIME, &tsafter) == -1) {
        perror("Error in clock_gettime()\n");
        return PTS_UNRESOLVED;
    }

    /*
     * Generic alg for calculating slept time.
     */
    slepts = tsafter.tv_sec-tsbefore.tv_sec;
    sleptns = tsafter.tv_nsec-tsbefore.tv_nsec;
    if (sleptns < 0) {
        sleptns = sleptns+1000000000;
        slepts = slepts-1;
    }

    if ((slepts > 0) || (sleptns > sleepnsec)) {
        printf("Test PASSED\n");
        return PTS_PASS;
    } else {
        printf("usleep() did not sleep long enough\n");
        return PTS_FAIL;
    }

    printf("This code should not be executed.\n");
    return PTS_UNRESOLVED;
}