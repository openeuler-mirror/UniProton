#define _BSD_SOURCE
#ifdef _GNU_SOURCE
#define HAD_SOURCE
#undef _GNU_SOURCE
#endif
#include <dirent.h>
#ifdef HAD_SOURCE
#undef HAD_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <stddef.h>
#include "__dirent.h"
#include "syscall.h"

#ifdef OS_OPTION_PROXY
#include "prt_proxy_ext.h"
#endif

typedef char dirstream_buf_alignment_check[1-2*(int)(
	offsetof(struct __dirstream, buf) % sizeof(off_t))];

struct dirent *readdir(DIR *dir)
{
	struct dirent *de;
	if (dir->buf_pos >= dir->buf_end) {
#ifdef OS_OPTION_PROXY
		int len = PRT_ProxyGetDents64(dir->fd, dir->buf, sizeof dir->buf);
#else
		int len = -ENOTSUP;
#endif
		if (len <= 0) {
			if (len < 0 && len != -ENOENT) errno = -len;
			return 0;
		}
		dir->buf_end = len;
		dir->buf_pos = 0;
	}
	de = (void *)(dir->buf + dir->buf_pos);
	dir->buf_pos += de->d_reclen;
	dir->tell = de->d_off;
	return de;
}

#ifndef OS_OPTION_PROXY
weak_alias(readdir, readdir64);
#endif
