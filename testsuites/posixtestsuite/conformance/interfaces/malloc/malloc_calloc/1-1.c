#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#include "posixtest.h"
#include "prt_fscmem.h"

#define TNAME "malloc_calloc/1-1.c"
#define TEST_SIZE 100

int mem_zero_detect(void *buf, size_t n)
{
    size_t size = n;
    if (size == 0) 
{
        return 0;
    }
    unsigned char *ptr = (unsigned char *)buf;
    if (*ptr == 0 && memcmp(ptr, ptr + 1, size - 1) == 0) {
      return 0;
    }
    return -1;
}

int malloc_calloc_1_1()
{
    size_t n = TEST_SIZE;
    unsigned int *ptr = calloc(sizeof(int), n);
    if (ptr == NULL) {
        printf(TNAME " Error at calloc(): calloc failed.\n");
        return PTS_FAIL;
    }

    if (mem_zero_detect(ptr, sizeof(int) * n) != 0) {
        PTS_FREE(ptr);
        printf ("Test FAILED\n");
        return PTS_FAIL;		
    }

    PTS_FREE(ptr);
    printf("Test PASSED\n");
    return PTS_PASS;
}

