#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <fcntl.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include "syscall.h"

int posix_fadvise(int fd, off_t base, off_t len, int advice)
{
    return ENOTSUP;
}

weak_alias(posix_fadvise, posix_fadvise64);
