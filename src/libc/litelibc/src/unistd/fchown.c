#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int fchown(int fd, uid_t uid, gid_t gid)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fchown(fd, uid, gid);
#else
    errno = ENOTSUP;
    return -1;
#endif
}
