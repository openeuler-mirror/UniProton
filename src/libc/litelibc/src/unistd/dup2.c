#include <errno.h>
#include <fcntl.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int dup2(int old, int new)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_dup2(old, new);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
