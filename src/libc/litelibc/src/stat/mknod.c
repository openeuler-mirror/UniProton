#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "syscall.h"
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

int mknod(const char *path, mode_t mode, dev_t dev)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_mknod(path, mode, dev);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
