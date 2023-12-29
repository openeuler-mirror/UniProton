#include "stdio_impl.h"
#include <sys/ioctl.h>
#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/fs/ioctl.h>
#endif

size_t __stdout_write(FILE *f, const unsigned char *buf, size_t len)
{
#ifdef OS_OPTION_NUTTX_VFS
	struct winsize wsz;
	f->write = __stdio_write;
	if (!(f->flags & F_SVB) && sys_ioctl(f->fd, TIOCGWINSZ, &wsz))
		f->lbf = -1;
	return __stdio_write(f, buf, len);
#else
	return 0;
#endif /* OS_OPTION_NUTTX_VFS */
}
