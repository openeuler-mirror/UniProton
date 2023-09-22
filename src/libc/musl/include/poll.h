#ifndef	_POLL_H
#define	_POLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>
#include <stdbool.h>
#include <stdint.h>

#include <bits/poll.h>

#define POLLIN     0x001
#define POLLPRI    0x002
#define POLLOUT    0x004
#define POLLERR    0x008
#define POLLHUP    0x010
#define POLLNVAL   0x020
#define POLLRDNORM 0x040
#define POLLRDBAND 0x080
#ifndef POLLWRNORM
#define POLLWRNORM 0x100
#define POLLWRBAND 0x200
#endif
#ifndef POLLMSG
#define POLLMSG    0x400
#define POLLRDHUP  0x2000
#endif

typedef unsigned long nfds_t;
typedef uint32_t pollevent_t;

struct pollfd;
typedef void (*pollcb_t)(struct pollfd *fds);
struct pollfd {
	int fd;
	short events;
	short revents;

    /* Non-standard fields used internally by NuttX. */

    void        *arg;       /* The poll callback function argument */
    pollcb_t    cb;         /* The poll callback function */
    void        *priv;      /* For use by drivers */
};

int poll (struct pollfd *, nfds_t, int);

/**
 *  From Nuttx sys/poll.h
 * */
int poll_fdsetup(int fd, struct pollfd *fds, bool setup);
void poll_default_cb(struct pollfd *fds);
void poll_notify(struct pollfd **afds, int nfds, pollevent_t eventset);

#ifdef _GNU_SOURCE
#define __NEED_time_t
#define __NEED_struct_timespec
#define __NEED_sigset_t
#include <bits/alltypes.h>
int ppoll(struct pollfd *, nfds_t, const struct timespec *, const sigset_t *);
#endif

#if _REDIR_TIME64
#ifdef _GNU_SOURCE
__REDIR(ppoll, __ppoll_time64);
#endif
#endif

#ifdef __cplusplus
}
#endif

#endif
