#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <stdio.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <stdlib.h>
#include "stdio_impl.h"

#ifdef OS_OPTION_NUTTX_VFS
#include "nuttx/sys/sys_fcntl.h"
#include "nuttx/sys/sys_unistd.h"
#endif

#define MAXTRIES 100

FILE *tmpfile(void)
{
#ifdef OS_OPTION_NUTTX_VFS
    char s[] = "/tmp/tmpfile_XXXXXX";
    int fd;
    FILE *f;
    int try;
    for (try=0; try<MAXTRIES; try++) {
        __randname(s+13);
        fd = sys_open(s, O_RDWR|O_CREAT|O_EXCL, 0600);
        if (fd >= 0) {
            sys_unlink(s);
            f = __fdopen(fd, "w+");
            if (!f) sys_close(fd);
            return f;
        }
    }
    return 0;
#else
    errno = ENOTSUP;
    return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}

weak_alias(tmpfile, tmpfile64);
