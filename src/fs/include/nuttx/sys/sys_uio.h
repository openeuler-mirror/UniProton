/****************************************************************************
 * include/nuttx/sys/sys_uio.h
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
 * Description: 适配UniProton和musl libc，使musl接口可以直接调用
 ****************************************************************************/

#ifndef __INCLUDE_SYS_UIO_H
#define __INCLUDE_SYS_UIO_H

#include <sys/uio.h>

ssize_t sys_readv(int fildes, const struct iovec *iov, int iovcnt);

ssize_t sys_writev(int fildes, const struct iovec *iov, int iovcnt);

#endif /* __INCLUDE_SYS_UIO_H */
