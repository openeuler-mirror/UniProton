#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <unistd.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int ftruncate(int fd, off_t length)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_ftruncate(fd, length);
#else
    errno = ENOTSUP;
    return -1;
#endif
}

weak_alias(ftruncate, ftruncate64);
