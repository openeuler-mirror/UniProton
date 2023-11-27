#include <assert.h>

_Noreturn void __assert_fail(const char *expr, const char *file, int line, const char *func)
{
    while (1) { }
}