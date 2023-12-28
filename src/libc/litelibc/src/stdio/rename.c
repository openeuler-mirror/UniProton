#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include "syscall.h"

#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

int rename(const char *old, const char *new)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_rename(old, new);
#endif
    errno = ENOTSUP;
    return -1;
}
