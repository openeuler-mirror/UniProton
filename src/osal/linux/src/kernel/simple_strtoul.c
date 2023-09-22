#include <stdlib.h>
#include <linux/kernel.h>

unsigned long simple_strtoul(const char *cp, char **endp, unsigned int base)
{
    return strtoul(cp, endp, (int)base);
}