#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <stdio.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include "stdio_impl.h"

int fgetpos(FILE *restrict f, fpos_t *restrict pos)
{
    off_t off = __ftello(f);
    if (off < 0) return -1;
    *(long long *)pos = off;
    return 0;
}

weak_alias(fgetpos, fgetpos64);
