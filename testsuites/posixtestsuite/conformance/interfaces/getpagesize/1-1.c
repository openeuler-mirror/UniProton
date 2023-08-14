#include <stdio.h>
#include <unistd.h>
#include "posixtest.h"

int getpagesize_1_1(int argc, char *argv[])
{
    int size = 0;
    size = getpagesize();
    if (size == 0) {
        perror("Error in getpagesize()\n");
        return PTS_UNRESOLVED;
    } else if (size == -1) {
        printf("getpagesize() not implement\n");
        return PTS_PASS;
    } else if (size > 0) {
        printf("Test PASSED\n");
        return PTS_PASS;
    }
    printf("This code should not be executed.\n");
    return PTS_UNRESOLVED;
}