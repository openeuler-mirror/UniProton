#include "stdio_impl.h"
#include <string.h>

size_t fwrite(const void *restrict src, size_t size, size_t nmemb, FILE *restrict f)
{
    size_t k, l = size*nmemb;
    if (!size) nmemb = 0;
    FLOCK(f);
    k = __fwritex(src, l, f);
    FUNLOCK(f);
    return k==l ? nmemb : k/size;
}

weak_alias(fwrite, fwrite_unlocked);
