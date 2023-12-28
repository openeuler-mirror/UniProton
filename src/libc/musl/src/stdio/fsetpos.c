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

int fsetpos(FILE *f, const fpos_t *pos)
{
    return __fseeko(f, *(const long long *)pos, SEEK_SET);
}

weak_alias(fsetpos, fsetpos64);
