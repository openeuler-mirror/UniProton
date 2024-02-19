#include <unistd.h>
#include <errno.h>

int setgid(gid_t gid)
{
    errno = ENOTSUP;
    return -1;
}
