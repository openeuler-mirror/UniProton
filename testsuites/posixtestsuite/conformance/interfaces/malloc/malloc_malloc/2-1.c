#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_malloc/2-1.c"

int malloc_malloc_2_1()
{
    size_t mallocSize = (OS_FSC_MEM_MAXVAL - OS_FSC_MEM_USED_HEAD_SIZE) - OS_FSC_MEM_TAIL_SIZE;
    void *ptr = malloc(mallocSize);

    if (ptr != NULL) {
        PTS_FREE(ptr);
        printf(TNAME " Error at malloc(): malloc failed.\n");
        return PTS_FAIL;
    }

    printf("Test PASSED\n");
    return PTS_PASS;
}

