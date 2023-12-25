#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int fsync(int fd)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_fsync(fd);
#else
    errno = ENOTSUP;
	return -1;
#endif
}
