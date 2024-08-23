#ifndef __INCLUDE_FS_PROXY_H
#define __INCLUDE_FS_PROXY_H

#include <stdarg.h>
#include <fcntl.h>
#include <stdbool.h>

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_PROXY)
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#define MAX_FDS 50
#define FILE_FDS_SIZE 1500
#define SOCKET_FDS_SIZE 1024

typedef struct file_record {
    int fd;
    bool isProxy;
} file_record_t;

bool proxyPath(const char *path);
void fds_init();
int fds_find(int fd);
int fds_get();
int vfs_fd_ctl(const char *path);

extern int fds_index[MAX_FDS][2];
extern file_record_t fds_record[MAX_FDS];
extern int PRT_ProxyFcntl(int fd, int cmd, ...);
extern int PRT_ProxyDup2(int oldfd, int newfd);
extern int PRT_ProxyClose(int fd);
extern ssize_t PRT_ProxyWrite(int fd, const void *buf, size_t count);
extern ssize_t PRT_ProxyReadLoop(int fd, void *buf, size_t count);
extern int PRT_ProxyOpen(const char *filename, int flags, ...);
extern off_t PRT_ProxyLseek(int fd, off_t offset, int whence);
extern int PRT_ProxyIoctl(int fd, unsigned long request, void *arg, size_t len);
extern int PRT_ProxyFstat(int fd, struct stat *restrict statbuf);
extern ssize_t PRT_ProxyReadLink(const char *pathname, char *buf, size_t bufsiz);
extern int PRT_ProxyRename(const char *old, const char *new);
extern int PRT_ProxyRmdir(const char *path);
extern int PRT_ProxyStat(const char *restrict pathname, struct stat *restrict statbuf);
extern int PRT_ProxyLstat(const char *restrict pathname, struct stat *restrict statbuf);
extern int PRT_ProxyUnlink(const char *path);
extern int PRT_ProxyMkdir(const char *pathname, mode_t mode);
#endif

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_PROXY) && defined(OS_SUPPORT_NET)
extern int PRT_CoexistClose(int s);
#endif

#endif /* __INCLUDE_FS_PROXY_H */