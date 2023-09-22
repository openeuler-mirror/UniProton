#include "test.h"
#include <stdlib.h>

static struct {
    int x, y, div, mod;
} t[] = {
2147483647, 2147483646, 1, 1,
2147483647, 2147483647, 1, 0,
2147483647, 536870908, 4, 15,
-2147483648, 2147483647, -1, -1,
-2147483647, -1, 2147483647, 0,
2147483647, 1, 2147483647, 0,
};

int div_test(void)
{
    int x, y;
    int i;
    div_t divResult;

    for (i = 0; i < sizeof t/sizeof *t; i++) {
        x = t[i].x;
        y = t[i].y;
        divResult = div(x, y);

        if (divResult.quot != t[i].div)
            t_error("div %d/%d want %d got %d\n", x, y, t[i].div, divResult.quot);
        if (divResult.rem != t[i].mod)
            t_error("mod %d%%%d want %d got %d\n", x, y, t[i].mod, divResult.rem);
    }
    return t_status;
}
