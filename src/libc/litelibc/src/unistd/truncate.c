#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <unistd.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <fcntl.h>

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#include "nuttx/sys/sys_fcntl.h"
#endif

int truncate(const char *path, off_t length)
{
#ifdef OS_OPTION_NUTTX_VFS
    int fd;
    int ret;

    DEBUGASSERT(path != NULL && length >= 0);

    /* Open the regular file at 'path' for write-only access */

    fd = sys_open(path, O_WRONLY);
    if (fd < 0) {
        return ERROR;
    }

    /* Then let ftruncate() do the work */

    ret = sys_ftruncate(fd, length);

    sys_close(fd);
    return ret;
#else
    errno = ENOTSUP;
    return -1;
#endif
}

weak_alias(truncate, truncate64);
