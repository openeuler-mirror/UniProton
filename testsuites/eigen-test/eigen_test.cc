#include <stdint.h>
#include <stdio.h>

#define FUNCTION_TEST(f) do{ f(); printf(#f " finish\n"); } while(0)

// extern void test_class_block();

extern "C" {

void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    (void)(param1);
    (void)(param2);
    (void)(param3);
    (void)(param4);

    printf("test start\n");
    // FUNCTION_TEST(test_class_block);
    printf("test end\n");
}
}
