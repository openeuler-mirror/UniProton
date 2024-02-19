#include <unistd.h>
#include <errno.h>

int setpgid(pid_t pid, pid_t pgid)
{
    errno = ENOTSUP;
    return -1;
}
