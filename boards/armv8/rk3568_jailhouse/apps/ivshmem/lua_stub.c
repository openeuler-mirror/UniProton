#include <string.h>
#include <stdarg.h>
#include <stdio.h>

char *setlocale(int cat, const char *name)
{
    return 0;
}

int setvbuf(FILE *stream, char *buffer, int mode, size_t size)
{
    return 0;
}

int fflush(FILE *stream)
{
    return 0;
}

int system(const char *cmd)
{
    return -1;
}

void (*signal(int sig, void (*func)(int)))(int)
{
    // lua.c使用，可不适配
    return func;
}

int isatty(int fd)
{
    return 0;
}

void flockfile(FILE *f)
{
    return;
}

void funlockfile(FILE *f)
{
    return;
}
