#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "securec.h"

#define OS_MAX_SHOW_LEN 256

extern int send_message(unsigned char *message, int len);
extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

int Test_Printf(const char *format, va_list vaList)
{
    int len;
    char buff[OS_MAX_SHOW_LEN] = {0};

    len = vsnprintf(buff, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }

    return send_message(buff, len);
}

int printf(const char *format, ...)
{
    int len = 0;
    va_list vaList;

    va_start(vaList, format);
    len = Test_Printf(format, vaList);
    va_end(vaList);

    return len;
}

void perror(const char *msg)
{
    char *errstr = strerror(errno);

    if (msg && *msg) {
        printf("%s: %s\n", msg, errstr);
    }
}

int puts(const char *s)
{
    return printf("%s\n", s);
}