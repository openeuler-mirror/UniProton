#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include <stdlib.h>
#include "syscall.h"
#include "kstat.h"

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_stat.h"
#endif

#define MAXTRIES 100

char *tempnam(const char *dir, const char *pfx)
{
#ifdef OS_OPTION_NUTTX_VFS
    char s[PATH_MAX];
    size_t l, dl, pl;
    int try;
    int r;

    if (!dir) dir = P_tmpdir;
    if (!pfx) pfx = "temp";

    dl = strlen(dir);
    pl = strlen(pfx);
    l = dl + 1 + pl + 1 + 6;

    if (l >= PATH_MAX) {
        errno = ENAMETOOLONG;
        return 0;
    }

    memcpy(s, dir, dl);
    s[dl] = '/';
    memcpy(s+dl+1, pfx, pl);
    s[dl+1+pl] = '_';
    s[l] = 0;

    for (try=0; try<MAXTRIES; try++) {
        __randname(s+l-6);
        r = sys_lstat(s, &(struct stat){0});
        if (r == -1 && errno == ENOENT) return strdup(s);
    }
    return 0;
#else
    errno = ENOTSUP;
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}
