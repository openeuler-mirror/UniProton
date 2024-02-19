#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int symlinkat(const char *existing, int fd, const char *new)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_symlinkat(existing, fd, new);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
