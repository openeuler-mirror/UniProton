#include <unistd.h>
#include <errno.h>

int setegid(gid_t egid)
{
    errno = ENOTSUP;
    return -1;
}
