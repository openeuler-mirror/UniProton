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

int getw(FILE *f)
{
    int x;
    return fread(&x, sizeof x, 1, f) ? x : EOF;
}
