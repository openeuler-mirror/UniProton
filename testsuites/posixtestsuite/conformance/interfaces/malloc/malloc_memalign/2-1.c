#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"
// XOPEN_SOURCE 没有定义memalign接口
#include "malloc.h"

#define TNAME "malloc_memalign/2-1.c"
#define MALLOC_SIZE 100

int malloc_memalign_2_1()
{
    for (size_t alignIndex = MEM_ADDR_ALIGN_004; alignIndex < MEM_ADDR_BUTT; alignIndex++) {
        void *ptr = memalign(1UL << alignIndex, MALLOC_SIZE);
        if (ptr == NULL) {
            printf(TNAME " Error at memalign(): memalign failed.\n");
            return PTS_UNRESOLVED;
        }
        if ((unsigned int)ptr % (1U << alignIndex) != 0) {
            printf(TNAME " Error at memalign() when alloc aligned with %u bytes(Addr: 0x%x)\n",
                   (1U << alignIndex), (unsigned int)ptr);
            PTS_FREE(ptr);
            return PTS_FAIL;
        }
        PTS_FREE(ptr);
    }

    printf("Test PASSED\n");
    return PTS_PASS;
}

