#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "securec.h"

#define OS_MAX_SHOW_LEN 0x200

extern int send_message(unsigned char *message, int len);
extern int vsnprintf(char *str, size_t size, const char *format, va_list ap);

// 在linux pts终端中打印
int printf(const char *format, ...)
{
    int len;
    va_list vaList;
    unsigned char buff[OS_MAX_SHOW_LEN];

    memset_s(buff, OS_MAX_SHOW_LEN, 0, OS_MAX_SHOW_LEN);
    va_start(vaList, format);
    len = vsnprintf(buff, OS_MAX_SHOW_LEN, format, vaList);
    if (len == -1) {
        return len;
    }
    va_end(vaList);

    return send_message(buff, len + 1);
}
