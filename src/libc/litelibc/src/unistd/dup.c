#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int dup(int fd)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_dup(fd);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
