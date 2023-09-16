#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_malloc/3-1.c"
#define MALLOC_SIZE_MAX ((OS_FSC_MEM_MAXVAL - OS_FSC_MEM_USED_HEAD_SIZE) - OS_FSC_MEM_TAIL_SIZE)

int malloc_malloc_3_1()
{
    size_t mallocSize = MALLOC_SIZE_MAX / 0xffffff;
    void *ptr1 = malloc(mallocSize);
    if (ptr1 == NULL) {
        printf(TNAME " Error at malloc(): malloc ptr1 failed.\n");
        return PTS_UNRESOLVED;
    }

    void *ptr2 = malloc(mallocSize * (0xffffff - 1));
    if (ptr2 != NULL) {
        PTS_FREE(ptr1);
        PTS_FREE(ptr2);
        printf(TNAME " Error at malloc(): malloc ptr2 failed.\n");
        return PTS_FAIL;
    }

    PTS_FREE(ptr1);
    printf("Test PASSED\n");
    return PTS_PASS;
}


