#include "riscv.h"


U64 r_mie()
{
    U64 x;
    OS_EMBED_ASM("csrr %0, mie":"=r"(x)::);
    return x;
}

U64 r_mstatus()
{
    U64 x;
    OS_EMBED_ASM("csrr %0, mstatus":"=r"(x)::);
    return x;
}

void w_mie(U64 x)
{
    OS_EMBED_ASM("csrw mie, %0"::"r"(x):);
}

void w_mstatus(U64 x)
{
    OS_EMBED_ASM("csrw mstatus, %0"::"r"(x):);
}
