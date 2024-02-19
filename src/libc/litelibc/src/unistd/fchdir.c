#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int fchdir(int fd)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fchdir(fd);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
