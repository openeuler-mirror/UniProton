#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

ssize_t readlinkat(int fd, const char *restrict path, char *restrict buf, size_t bufsize)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_readlinkat(fd, path, buf, bufsize);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
