#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_realloc/2-1.c"
#define MALLOC_SIZE_NEW 120

int malloc_realloc_2_1()
{
    U32 *newPtr = realloc(NULL, sizeof(U32) * MALLOC_SIZE_NEW);
    if (!newPtr) {
        printf(TNAME " Error at realloc(): realloc failed.\n");
        return PTS_FAIL;
    }

    printf("Test PASSED\n");
    PTS_FREE(newPtr);
    return PTS_PASS;
}

