#include "libc.h"

struct __libc __libc;

size_t __hwcap;
char *__progname=0, *__progname_full=0;

#if defined(OS_OPTION_LOCALE)
const void *libc_global_locale = &libc.global_locale;
#endif

weak_alias(__progname, program_invocation_short_name);
weak_alias(__progname_full, program_invocation_name);
