#include <unistd.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int fchownat(int fd, const char *path, uid_t uid, gid_t gid, int flag)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fchownat(fd, path, uid, gid, flag);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
