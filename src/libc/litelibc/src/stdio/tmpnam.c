#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include "kstat.h"

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

#define MAXTRIES 100

char *tmpnam(char *buf)
{
#ifdef OS_OPTION_NUTTX_VFS
    static char internal[L_tmpnam];
    char s[] = "/tmp/tmpnam_XXXXXX";
    int try;
    int r;
    for (try=0; try<MAXTRIES; try++) {
        __randname(s+12);
        r = sys_lstat(s, &(struct stat){0});
        if (r == -1 && errno == ENOENT) return strcpy(buf ? buf : internal, s);
    }
    return 0;
#else
    errno = ENOTSUP;
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}
