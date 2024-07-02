/****************************************************************************
 * include/nuttx/sys/sys_fcntl.h
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

#ifndef __INCLUDE_FS_FCNTL_H
#define __INCLUDE_FS_FCNTL_H

#include <stdarg.h>
#include <fcntl.h>
#include <stdbool.h>

int sys_open(const char *path, int oflags, ...);
int sys_fcntl(int fd, int cmd, ...);
int sys_openat(int dirfd, const char *path, int oflags, ...);
int sys_posix_fallocate(int fd, off_t offset, off_t len);

#endif /* __INCLUDE_FS_FCNTL_H */
