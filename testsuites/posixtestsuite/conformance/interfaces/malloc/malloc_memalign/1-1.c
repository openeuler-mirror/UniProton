#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_memalign/1-1.c"
#define MALLOC_SIZE 100

int malloc_memalign_1_1()
{
    size_t mallocSize = MALLOC_SIZE;
    void *ptr1 = memalign(MEM_ADDR_ALIGN_004 -1, mallocSize);
    if (ptr1 != NULL) {
        PTS_FREE(ptr1);
        printf(TNAME " Error at memalign(): memalign ptr1 failed.\n");
        return PTS_FAIL;
    }

    void *ptr2 = memalign(MEM_ADDR_BUTT, mallocSize);
    if (ptr2 != NULL) {
        PTS_FREE(ptr2);
        printf(TNAME " Error at memalign(): memalign ptr2 failed.\n");
        return PTS_FAIL;
    }

    printf("Test PASSED\n");
    return PTS_PASS;
}


