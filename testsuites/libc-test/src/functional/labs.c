#include "stdlib.h"
#include "test.h"

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    long originValue;
    long absoluteValue;
} t[] = {
    {-1, 1},
    {-0, 0},
    {0, 0},
    {2147483647, 2147483647},
    {-2147483647, 2147483647},
};

int labs_test(void)
{
    int i;
    for (i = 0; i < length(t); i++) {
        if (t[i].absoluteValue != labs(t[i].originValue))
            t_error("labs(\"%ld\") got %ld\n", t[i].originValue, t[i].absoluteValue);
    }

    return t_status;
}