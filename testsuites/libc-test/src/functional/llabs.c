#include "stdlib.h"
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    long long originValue;
    long long absoluteValue;
} t[] = {
    {-1, 1},
    {-0, 0},
    {0, 0},
    {9223372036854775807LL, 9223372036854775807LL},
    {-9223372036854775807LL, 9223372036854775807LL},
};

int llabs_test(void)
{
    int i;
    for (i = 0; i < length(t); i++) {
        if (t[i].absoluteValue != llabs(t[i].originValue))
            t_error("llabs(\"%lld\") got %lld\n", t[i].originValue, t[i].absoluteValue);
    }
    return t_status;
}