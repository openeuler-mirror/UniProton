#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <string.h>

char *strsep(char **str, const char *sep)
{
    char *s = *str, *end;
    if (!s) return NULL;
    end = s + strcspn(s, sep);
    if (*end) *end++ = 0;
    else end = 0;
    *str = end;
    return s;
}
