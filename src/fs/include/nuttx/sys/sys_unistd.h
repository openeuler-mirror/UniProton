/****************************************************************************
 * include/nuttx/sys/sys_unistd.h
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Description: 适配UniProton和musl libc，使musl接口可以直接调用，避免符号冲突
 ****************************************************************************/

#ifndef __INCLUDE_FS_UNISTD_H
#define __INCLUDE_FS_UNISTD_H

#include <unistd.h>
#include <nuttx/compiler.h>

int sys_close(int fd);

ssize_t sys_read(int fd, void *buf, size_t nbytes);

ssize_t sys_write(int fd, const void *buf, size_t nbytes);

off_t sys_lseek(int fd, off_t offset, int whence);

int sys_unlink(const char *pathname);

int sys_rmdir(const char *pathname);

int sys_rename(const char *oldpath, const char *newpath);

int sys_dup2(int fd1, int fd2);

int sys_chown(FAR const char *path, uid_t owner, gid_t group);

int sys_fsync(int fd);

int sys_dup(int fd);

ssize_t sys_pread(int fd, FAR void *buf, size_t nbytes, off_t offset);

ssize_t sys_pwrite(int fd, FAR const void *buf, size_t nbytes, off_t offset);

void sys_sync(void);

ssize_t sys_readlink(FAR const char *path, FAR char *buf, size_t bufsize);

ssize_t sys_readlinkat(int dirfd, FAR const char *path, FAR char *buf, size_t bufsize);

int sys_link(FAR const char *path1, FAR const char *path2);

int sys_fchown(int fd, uid_t owner, gid_t group);

int sys_lchown(FAR const char *path, uid_t owner, gid_t group);

int sys_symlink(FAR const char *path1, FAR const char *path2);

int sys_symlinkat(FAR const char *path1, int dirfd, FAR const char *path2);

int sys_ftruncate(int fd, off_t length);

char *sys_getcwd(FAR char *buf, size_t size);

int sys_pipe2(int fd[2], int flags);

int sys_unlinkat(int dirfd, FAR const char *path, int flags);

int sys_linkat(int olddirfd, FAR const char *path1, int newdirfd, FAR const char *path2, int flags);

int sys_fchownat(int dirfd, FAR const char *path, uid_t owner, gid_t group, int flags);

int sys_access(FAR const char *path, int amode);

int sys_faccessat(int dirfd, FAR const char *path, int amode, int flags);

int sys_chdir(FAR const char *path);

int sys_fchdir(int fd);

#endif /* __INCLUDE_FS_UNISTD_H */
