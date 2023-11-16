#ifndef PRT_POSIX_EXT_H
#define PRT_POSIX_EXT_H

#include <locale.h>

locale_t *PRT_LocaleCurrent();

extern const void *libc_global_locale;
#endif