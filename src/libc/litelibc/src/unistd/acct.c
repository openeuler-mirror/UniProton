#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <unistd.h>
#include <errno.h>

int acct(const char *filename)
{
    errno = ENOTSUP;
    return -1;
}
