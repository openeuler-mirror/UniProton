#include <fcntl.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int symlink(const char *existing, const char *new)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_symlink(existing, new);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
