#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <string.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_unistd.h>
#endif

char *getcwd(char *buf, size_t size)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_getcwd(buf, size);
#else
    errno = ENOTSUP;
    return NULL;
#endif
}
