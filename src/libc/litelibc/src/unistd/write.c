#include <unistd.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

ssize_t write(int fd, const void *buf, size_t count)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_write(fd, buf, count);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
