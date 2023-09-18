#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_malloc/1-1.c"

int malloc_malloc_1_1()
{
    void *ptr = malloc(0);

    if (ptr != NULL) {
        PTS_FREE(ptr);
        printf(TNAME " Error at malloc(): malloc failed.\n");
        return PTS_FAIL;
    }

    printf("Test PASSED\n");
    return PTS_PASS;
}

