#include <unistd.h>
#include <errno.h>

int seteuid(uid_t euid)
{
    errno = ENOTSUP;
    return -1;
}
