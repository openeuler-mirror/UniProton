#include <stdio.h>
#include <stdarg.h>
#include "securec.h"

extern int send_message(unsigned char *message, int len);

#define OS_MAX_PRINT_LEN 0x200

int vprintf(const char *restrict fmt, va_list ap)
{
    int len;
    unsigned char buff[OS_MAX_PRINT_LEN];

    memset_s(buff, OS_MAX_PRINT_LEN, 0, OS_MAX_PRINT_LEN);
    len = vsnprintf((char *)buff, OS_MAX_PRINT_LEN, fmt, ap);
    if (len == -1) {
        return len;
    }

    return send_message(buff, len + 1);
}
