#include "stdio_impl.h"
#include <errno.h>
#include <unistd.h>

int pclose(FILE *f)
{
    errno = ENOTSUP;
    return -1;
}
