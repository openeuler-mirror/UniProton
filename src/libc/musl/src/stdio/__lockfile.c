#include "stdio_impl.h"
#include "pthread_impl.h"
#include <limits.h>

int __lockfile(FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
	long self = (long)pthread_self();

	if (f->owner == self) {
		if (f->lockcount == LONG_MAX) {
			return 0;
		}
		f->lockcount++;
		return 1;
	}

	if (pthread_mutex_lock(&f->mutex) == 0) {
		f->owner = self;
		f->lockcount = 1;
		return 1;
	}
#endif /* OS_OPTION_NUTTX_VFS */
	return 0;
}

void __unlockfile(FILE *f)
{
#ifdef OS_OPTION_NUTTX_VFS
	if (f->lockcount > 1) {
		f->lockcount--;
		return;
	}

	f->lockcount = 0;
	if (pthread_mutex_unlock(&f->mutex) == 0) {
		f->owner = -1;
	}
#endif /* OS_OPTION_NUTTX_VFS */
}
