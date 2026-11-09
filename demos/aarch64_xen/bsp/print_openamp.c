#include <stdarg.h>
#include <stdio.h>
#include <errno.h>
#include <mica/service.h>

int mica_tty_vprintf(const char *format, va_list ap)
{
    if (!format) return -EINVAL;
    unsigned char buffer[512];
    int len=vsnprintf((char *)buffer,sizeof(buffer),format,ap);
    if (len<0) return len;
    if ((size_t)len>=sizeof(buffer)) return -EMSGSIZE;
    return len?mica_tty_send(buffer,(size_t)len):0;
}
/* This translation unit is private to aarch64_xen, never the early console. */
int printf(const char *format, ...)
{
    va_list ap;
    va_start(ap,format);
    int ret=mica_tty_vprintf(format,ap);
    va_end(ap);
    return ret;
}
