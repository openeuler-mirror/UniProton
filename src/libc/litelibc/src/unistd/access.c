#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int access(const char *filename, int amode)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_access(filename, amode);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
