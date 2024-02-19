#include <unistd.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int fdatasync(int fd)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fsync(fd);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
