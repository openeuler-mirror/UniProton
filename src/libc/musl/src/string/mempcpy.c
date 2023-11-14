#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>

void *mempcpy(void *dest, const void *src, size_t n)
{
    return (char *)memcpy(dest, src, n) + n;
}
