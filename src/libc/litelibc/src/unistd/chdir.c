#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int chdir(const char *path)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_chdir(path);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
