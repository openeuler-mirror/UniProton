/****************************************************************************
 * include/nuttx/fs/fs_poll.h
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

#ifndef __INCLUDE_FS_POLL_H
#define __INCLUDE_FS_POLL_H

#include <poll.h>

struct __pollfd;

typedef uint32_t pollevent_t;
typedef void (*pollcb_t)(struct __pollfd *fds);

struct __pollfd {
    struct pollfd pollfd;

    void        *arg;       /* The poll callback function argument */
    pollcb_t    cb;         /* The poll callback function */
    void        *priv;      /* For use by drivers */
};

int poll_fdsetup(int fd, struct __pollfd *fds, bool setup);
void poll_default_cb(struct __pollfd *fds);
void poll_notify(struct __pollfd **afds, int nfds, pollevent_t eventset);
int fs_poll(struct pollfd *fds, nfds_t nfds, int timeout);

#endif /* __INCLUDE_FS_POLL_H */
