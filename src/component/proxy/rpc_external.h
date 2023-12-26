/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *     http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * See the Mulan PSL v2 for more details.
 * Create: 2023-06-30
 * Description: 代理接口对外头文件
 */

#ifndef _RPC_EXTERNAL_H
#define _RPC_EXTERNAL_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>

#include "rpc_internal_common.h"
#include "rpc_internal_model.h"
#include "rpc_client_internal.h"
#include "rpc_err.h"
#include "prt_buildef.h"
#include "prt_queue.h"
#include "prt_proxy_ext.h"

int PRT_ProxyOpen(const char *filename, int flags, ...);
ssize_t PRT_ProxyReadLoop(int fd, void *buf, size_t count);
ssize_t PRT_ProxyWrite(int fd, const void *buf, size_t count);
int PRT_ProxyClose(int fd);
int PRT_ProxyFcntl(int fd, int cmd, ...);
 off_t PRT_ProxyLseek(int fd, off_t offset, int whence);
int PRT_ProxyUnlink(const char *path);
void PRT_ProxyFreeAddrInfo(struct addrinfo *ai);
int PRT_ProxyGetAddrInfo(const char *node, const char *service,
    const struct addrinfo *hints, struct addrinfo **res);
struct hostent *PRT_ProxyGetHostByAddr(const void *addr, socklen_t len, int type);
struct hostent *PRT_ProxyGetHostByName(const char *name);
int PRT_ProxyPoll(struct pollfd *fds, nfds_t nfds, int timeout);
int PRT_ProxyGetPeerName(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int PRT_ProxyGetHostName(char *name, size_t len);
int PRT_ProxyGetSockName(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int PRT_ProxyGetSockOpt(int sockfd, int level, int optname, void *optval,
    socklen_t *optlen);
int PRT_ProxySelect(int n, fd_set *restrict rfds, fd_set *restrict wfds,
    fd_set *restrict efds, struct timeval *restrict tv);
int PRT_ProxyAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int PRT_ProxyBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int PRT_ProxyConnect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int PRT_ProxyListen(int sockfd, int backlog);
ssize_t PRT_ProxyRecv(int sockfd, void *buf, size_t len, int flags);
ssize_t PRT_ProxyRecvFrom(int sockfd, void *buf, size_t len, int flags,
    struct sockaddr *src_addr, socklen_t *addrlen);
ssize_t PRT_ProxySend(int sockfd, const void *buf, size_t len, int flags);
ssize_t PRT_ProxySendTo(int sockfd, const void *buf, size_t len, int flags,
    const struct sockaddr *dest_addr, socklen_t addrlen);
int PRT_ProxySetSockOpt(int sockfd, int level, int optname, const void *optval,
    socklen_t optlen);
int PRT_ProxyShutdown(int sockfd, int how);
int PRT_ProxySocket(int domain, int type, int protocol);
int PRT_ProxyWriteStdOut(const char *buf, int len);
int PRT_ProxyVprintf(const char *format, va_list list);
int PRT_ProxyPrintf(const char *format, ...);
int PRT_ProxyGetDents64(int fd, char *buf, int len);
FILE *PRT_ProxyFopen(const char *filename, const char *mode);
int PRT_ProxyFclose(FILE *f);
size_t PRT_ProxyFreadLoop(void *buffer, size_t size, size_t count, FILE *f);
size_t PRT_ProxyFwriteLoop(const void *buffer, size_t size, size_t count, FILE *f);
FILE *PRT_ProxyFreopen(const char *filename, const char *mode, FILE *f);
int PRT_ProxyFputs(const char *str, FILE *f);
char *PRT_ProxyFgets(char *str, int n, FILE *f);
int PRT_ProxyFeof(FILE *f);
int PRT_ProxyFprintf(FILE *f, const char *format, ...);
int PRT_ProxyVfprintf(FILE *restrict f, const char *restrict fmt, va_list ap);
int PRT_ProxyGetc(FILE *f);
int PRT_ProxyFerror(FILE *f);
int PRT_ProxyGetcUnlocked(FILE *f);
int PRT_ProxyPclose(FILE *f);
FILE *PRT_ProxyTmpfile(void);
void PRT_ProxyClearerr(FILE *f);
FILE *PRT_ProxyPopen(const char *cmd, const char *mode);
int PRT_ProxyUngetc(int c, FILE *f);
int PRT_ProxyFseeko(FILE *f, off_t offset, int whence);
long PRT_ProxyFtello(FILE * f);
int PRT_ProxyRename(const char *old, const char *new);
int PRT_ProxyRemove(const char *path);
int PRT_ProxyMkstemp(char *template);
int PRT_ProxyFflush(FILE *f);
wint_t PRT_ProxyGetwc(FILE *f);
wint_t PRT_ProxyPutwc(wchar_t wc, FILE *f);
int PRT_ProxyPutc(int c, FILE *f);
wint_t PRT_ProxyUngetwc(wint_t wc, FILE *f);
int PRT_ProxyStat(const char *restrict pathname, struct stat *restrict statbuf);
int PRT_ProxyLstat(const char *restrict pathname, struct stat *restrict statbuf);
char *PRT_ProxyGetcwd(char *buf, size_t size);
int PRT_ProxyFstat(int fd, struct stat *restrict statbuf);
FILE *PRT_ProxyFdopen(int fd, const char *mode);
int PRT_ProxyFileno(FILE *f);
int PRT_ProxySetvbuf(FILE *f, char *buf, int mode, size_t size);
int PRT_ProxySystem(const char *command);
ssize_t PRT_ProxyReadLink(const char *pathname, char *buf, size_t bufsiz);
int PRT_ProxyAccess(const char *pathname, int mode);
int PRT_ProxyDup2(int oldfd, int newfd);
int PRT_ProxyMkfifo(const char *pathname, mode_t mode);
int PRT_ProxyChmod(const char *pathname, mode_t mode);
int PRT_ProxyChdir(const char *path);
int PRT_ProxyMkdir(const char *pathname, mode_t mode);
int PRT_ProxyRmdir(const char *path);
int PRT_ProxyPipe(int fd[2]);
int PRT_ProxyFscanfx(FILE *f, const char *fmt, ...);

#endif /* _RPC_EXTERNAL_H */