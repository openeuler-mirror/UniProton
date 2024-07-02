#include <nuttx/config.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sched.h>
#include <errno.h>
#include <assert.h>
#include <stdarg.h>

#ifdef OS_OPTION_NUTTX_VFS
#include <nuttx/sys/sys_fcntl.h>
#include <nuttx/sys/sys_unistd.h>
#include "nuttx/mutex.h"
#endif

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_PROXY)
#include "fs_proxy.h"
#endif
 
int fds_index[MAX_FDS][2] = {0};
static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;
file_record_t fds_record[MAX_FDS];

bool proxyPath(const char *path)
{
  char proxy_path[] = "/tmp/";
  int len = strlen(proxy_path);
  char tmp_path[len];

  strncpy_s(tmp_path, len + 1, path, len);
  int ret = strcmp(tmp_path, proxy_path);

  return (ret == 0 ? true:false);
}

void fds_init()
{
    for(int i = 0; i < MAX_FDS; i++) {
        fds_index[i][0] = FILE_FDS_SIZE + i + 1;
        fds_index[i][1] = 0;
        fds_record[i].fd = -1;
    }
}

int fds_find(int fd)
{
    int index = fd - 1 - FILE_FDS_SIZE;

    if(fds_index[index][1] == 1) {
        return index;
    }
    return -1;
}

int fds_get()
{
    for(int i = 0; i < MAX_FDS; i++) {
        if(fds_index[i][0] > 0 && fds_index[i][1] == 0) {
            return i;
        }
    }
    return -1;
}

int vfs_fd_ctl(const char *path)
{
    pthread_mutex_lock(&fd_lock);

    if(fds_index[0][0] == 0) {
        fds_init();
    }

    int index = fds_get();
    if(index < 0) {
        pthread_mutex_unlock(&fd_lock);
        return -EINVAL;
    }

    fds_index[index][1] = 1;
    pthread_mutex_unlock(&fd_lock);

    fds_record[index].isProxy = false;

    if (proxyPath(path)) {
        fds_record[index].isProxy = true;
    }

    return fds_index[index][0];
}