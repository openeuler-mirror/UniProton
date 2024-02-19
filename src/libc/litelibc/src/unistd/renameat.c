#include <stdio.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_stdio.h>
#endif

int renameat(int oldfd, const char *old, int newfd, const char *new)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_renameat(oldfd, old, newfd, new);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
