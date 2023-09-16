#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_realloc/1-1.c"
#define MALLOC_SIZE 100
#define MALLOC_SIZE_NEW 120

int malloc_realloc_1_1()
{
    U32 *oldPtr = malloc(sizeof(U32) * MALLOC_SIZE);
    if (!oldPtr) {
        printf(TNAME " Error at realloc(): malloc oldPtr failed.\n");
        return PTS_UNRESOLVED;
    }
    for (U32 index = 0; index < MALLOC_SIZE; index++) {
        *(U32 *)(oldPtr + index) = index + 1;
    }

    U32 *newPtr = realloc(oldPtr, sizeof(U32) * MALLOC_SIZE_NEW);
    if (!newPtr) {
        printf(TNAME " Error at realloc(): malloc newPtr failed.\n");
        PTS_FREE(oldPtr);
        return PTS_FAIL;
    }
    for (U32 index = 0; index < MALLOC_SIZE; index++) {
        if (*(U32 *)(newPtr + index) != index + 1) {
            printf(TNAME " Error: content of oldPtr and newPtr is inconsistent.\n");
            PTS_FREE(newPtr);
            return PTS_FAIL;	
        }
    }

    printf("Test PASSED\n");
    PTS_FREE(newPtr);
    return PTS_PASS;
}

