#include "locale_impl.h"
#include "pthread_impl.h"
#include "libc.h"
#include "prt_posix_ext.h"

locale_t __uselocale(locale_t new)
{
    locale_t *old = PRT_LocaleCurrent();
    locale_t global = &libc.global_locale;

    if (new) {
        *old = new == LC_GLOBAL_LOCALE ? global : new;
    }
    return *old == global ? LC_GLOBAL_LOCALE : *old;
}

weak_alias(__uselocale, uselocale);
