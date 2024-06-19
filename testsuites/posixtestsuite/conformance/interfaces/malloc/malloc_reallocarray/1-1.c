#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"
// XOPEN_SOURCE 没有定义reallocarray接口
#include "malloc.h"

#define TNAME "malloc_reallocarray/1-1.c"
#define MALLOC_SIZE 100
#define MALLOC_SIZE_NEW 120

int malloc_reallocarray_1_1()
{
    U32 *oldPtr = malloc(sizeof(U32) * MALLOC_SIZE);
    if (!oldPtr) {
        printf(TNAME " Error at reallocarray(): malloc failed.\n");
        return PTS_UNRESOLVED;
    }

    U32 *newPtr = reallocarray(oldPtr, sizeof(U32), MALLOC_SIZE_NEW);
    if (!newPtr) {
        printf(TNAME " Error at reallocarray(): reallocarray failed.\n");
        PTS_FREE(oldPtr);
        return PTS_FAIL;
    }

    printf("Test PASSED\n");
    PTS_FREE(newPtr);
    return PTS_PASS;
}

