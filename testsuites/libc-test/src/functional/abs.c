#include "stdlib.h"
#include "test.h"
#include <stdio.h>

#define length(x) (sizeof(x) / sizeof *(x))

static struct {
    int originValue;
    int absoluteValue;
} t[] = {
    {-1, 1},
    {-0, 0},
    {0, 0},
    {-2147483647, 2147483647},
    {2147483647, 2147483647},
};

int abs_test(void)
{
    int i;
    for (i = 0; i < length(t); i++) {
        if (t[i].absoluteValue != labs(t[i].originValue))
        {
            printf("labs(\"%d\") got %d\n", t[i].originValue, t[i].absoluteValue);
            t_status = 1;
        }
    }
    return t_status;
}