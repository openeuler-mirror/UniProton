#include <sys/stat.h>
#include <errno.h>
#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

mode_t umask(mode_t mode)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_umask(mode);
#else
    return -EAFNOSUPPORT;
#endif
}
