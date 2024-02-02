#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int chmod(const char *path, mode_t mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_chmod(path, mode);
#else
    errno = ENOTSUP;
    return -1;
#endif /* OS_OPTION_NUTTX_VFS */
}
