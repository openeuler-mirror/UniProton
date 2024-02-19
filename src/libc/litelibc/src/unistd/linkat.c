#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int linkat(int fd1, const char *existing, int fd2, const char *new, int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_linkat(fd1, existing, fd2, new, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
