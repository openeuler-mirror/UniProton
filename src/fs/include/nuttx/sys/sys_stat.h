/****************************************************************************
 * include/nuttx/sys/sys_stat.h
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

#ifndef __INCLUDE_SYS_STAT_H
#define __INCLUDE_SYS_STAT_H

#include <sys/stat.h>
#include <sys/statfs.h>
#include <stddef.h>

mode_t sys_umask(mode_t);

int sys_stat(const char *path, struct stat *buf);

int sys_lstat(const char *path, struct stat *buf);

int sys_fstat(int fd, struct stat *buf);

int sys_fstatat(int dirfd, const char *path, struct stat *buf, int flags);

int sys_chmod(const char *path, mode_t mode);

int sys_lchmod(const char *path, mode_t mode);

int sys_fchmod(int fd, mode_t mode);

int sys_mkdir(const char *pathname, mode_t mode);

int sys_mkdirat(int dirfd, const char *path, mode_t mode);

int sys_mknod(const char *path, mode_t mode, dev_t dev);

int sys_mknodat(int dirfd, const char *path, mode_t mode, dev_t dev);

int sys_mkfifo(const char *pathname, mode_t mode);

int sys_mkfifoat(int dirfd, const char *path, mode_t mode);

int sys_fchmodat(int dirfd, const char *path, mode_t mode, int flags);

int sys_utimensat(int dirfd, const char *path, const struct timespec times[2], int flags);

int sys_futimens(int fd, const struct timespec times[2]);

int sys_statfs(const char *path, struct statfs *buf);

int sys_fstatfs(int fd, struct statfs *buf);

int utimens(const char *path, const struct timespec times[2]);
int lutimens(const char *path, const struct timespec times[2]);
mode_t getumask(void);

int lib_getfullpath(int dirfd, const char *path, char *fullpath, size_t fulllen);

#endif /* __INCLUDE_SYS_STAT_H */
