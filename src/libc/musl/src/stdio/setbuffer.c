#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>

void setbuffer(FILE *f, char *buf, size_t size)
{
    setvbuf(f, buf, buf ? _IOFBF : _IONBF, size);
}
