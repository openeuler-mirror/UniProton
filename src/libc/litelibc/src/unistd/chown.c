#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

int chown(const char *path, uid_t uid, gid_t gid)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_chown(path, uid, gid);
#else
    errno = ENOTSUP;
	return -1;
#endif
}
