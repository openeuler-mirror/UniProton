#include "stdio_impl.h"
#include "pthread_impl.h"

int __lockfile(FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
	if (pthread_mutex_lock(&f->mutex) == 0) {
		f->owner = (long)pthread_self();
		return 1;
	}
#endif /* OS_OPTION_NUTTX_VFS */
	return 0;
}

void __unlockfile(FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
	if (pthread_mutex_unlock(&f->mutex) == 0) {
		f->owner = -1;
	}
#endif /* OS_OPTION_NUTTX_VFS */
}
