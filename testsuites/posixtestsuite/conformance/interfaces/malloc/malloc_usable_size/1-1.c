#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_usable_size/1-1.c"
#define MALLOC_SIZE 25

int malloc_usable_size_1_1()
{
    size_t usableSize = 0;
    void *ptr = malloc(MALLOC_SIZE);
    if (ptr == NULL) {
        printf(TNAME " Error at malloc_usable_size(): malloc failed.\n");
        return PTS_UNRESOLVED;
    }

    usableSize = malloc_usable_size(ptr);
    if (usableSize < MALLOC_SIZE) {
        PTS_FREE(ptr);
        printf(TNAME " Error at malloc_usable_size(): usableSize is too small.\n");
        return PTS_FAIL;
    }
    if (*(U32 *)((U8 *)ptr + usableSize) != OS_FSC_MEM_TAIL_MAGIC) {
        PTS_FREE(ptr);
        printf(TNAME " Error at malloc_usable_size(): usableSize or tail error.\n");
        return PTS_FAIL;
    }

    PTS_FREE(ptr);
    printf("Test PASSED\n");
    return PTS_PASS;
}

