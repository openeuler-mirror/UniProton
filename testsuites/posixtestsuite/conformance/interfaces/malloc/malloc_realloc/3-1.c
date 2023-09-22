#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_realloc/3-1.c"
#define MALLOC_SIZE 120
#define MALLOC_SIZE_NEW 0


int malloc_realloc_3_1()
{
    U32 *oldPtr = malloc(sizeof(unsigned int) * MALLOC_SIZE);
    if (!oldPtr) {
        printf(TNAME " Error at realloc(): malloc failed.\n");
        return PTS_UNRESOLVED;
    }

    U32 *newPtr = realloc(oldPtr, sizeof(U32) * MALLOC_SIZE_NEW);
    if (newPtr) {
        printf(TNAME " Error at realloc(): realloc failed.\n");
        PTS_FREE(newPtr);
        return PTS_FAIL;
    }

    printf("Test PASSED\n");
    PTS_FREE(newPtr);
    return PTS_PASS;
}


