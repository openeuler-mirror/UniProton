#include <unistd.h>
#include <errno.h>

int getgroups(int count, gid_t list[])
{
    errno = ENOTSUP;
    return -1;
}
