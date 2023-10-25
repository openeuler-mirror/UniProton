#define _GNU_SOURCE
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include "__dirent.h"
#include "syscall.h"

DIR *opendir(const char *name)
{
	int fd;
	DIR *dir;

	if ((fd = open(name, O_RDONLY|O_DIRECTORY|O_CLOEXEC)) < 0)
		return 0;
#ifdef OS_OPTION_PROXY
	if (!(dir = __libc_calloc(1, sizeof *dir))) {
		close(fd);
#else
	if (!(dir = calloc(1, sizeof *dir))) {
		__syscall(SYS_close, fd);
#endif
		return 0;
	}
	dir->fd = fd;
	return dir;
}
