#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>

void setlinebuf(FILE *f)
{
    setvbuf(f, 0, _IOLBF, 0);
}
