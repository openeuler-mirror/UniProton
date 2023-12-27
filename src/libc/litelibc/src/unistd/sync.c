#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_unistd.h"
#endif

void sync(void)
{
#ifdef OS_OPTION_NUTTX_VFS
    return sys_sync();
#endif
}
