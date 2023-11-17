#include <stdint.h>
#include <stdio.h>

extern int test_misc_1();

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)(param1);
    (void)(param2);
    (void)(param3);
    (void)(param4);

    test_misc_1();
    printf("test end\n");
}
}

