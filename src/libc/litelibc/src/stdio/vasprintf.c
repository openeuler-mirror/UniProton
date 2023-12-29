#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "stdio_impl.h"
#include <limits.h>
#include <errno.h>
#include <string.h>

#ifdef OS_OPTION_NUTTX_VFS
struct cookie {
    char *s;
    size_t n;
};

#define MIN(a, b) ((a) < (b) ? (a) : (b))

static size_t sn_write(FILE *f, const unsigned char *s, size_t l)
{
    struct cookie *c = f->cookie;
    size_t k = MIN(c->n, f->wpos - f->wbase);
    if (k) {
        memcpy(c->s, f->wbase, k);
        c->s += k;
        c->n -= k;
    }
    k = MIN(c->n, l);
    if (k) {
        memcpy(c->s, s, k);
        c->s += k;
        c->n -= k;
    }
    *c->s = 0;
    f->wpos = f->wbase = f->buf;
    /* pretend to succeed, even if we discarded extra data */
    return l;
}

static int vslen(char *restrict s, size_t n, const char *restrict fmt, va_list ap)
{
    unsigned char buf[1];
    char dummy[1];
    struct cookie c = { .s = n ? s : dummy, .n = n ? n-1 : 0 };
    FILE f = {
        .lbf = EOF,
        .write = sn_write,
        .buf = buf,
        .cookie = &c,
        .owner = -1,
        .mutex = {PTHREAD_MUTEX_RECURSIVE, 0, 0},
    };

    if (n > INT_MAX) {
        errno = EOVERFLOW;
        return -1;
    }

    *c.s = 0;
    return vfprintf(&f, fmt, ap);
}
#endif /* OS_OPTION_NUTTX_VFS */

int vasprintf(char **s, const char *fmt, va_list ap)
{
    va_list ap2;
    va_copy(ap2, ap);
#ifdef OS_OPTION_NUTTX_VFS
    int l = vslen(0, 0, fmt, ap2);
#else
    int l = vsnprintf(0, 0, fmt, ap2);
#endif /* OS_OPTION_NUTTX_VFS */
    va_end(ap2);

    if (l<0 || !(*s=malloc(l+1U))) return -1;
    return vsnprintf(*s, l+1U, fmt, ap);
}
