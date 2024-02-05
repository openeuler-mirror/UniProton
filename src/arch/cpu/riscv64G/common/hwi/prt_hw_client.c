#include "prt_hw_client.h"
#include "prt_cpu_external.h"
#include "prt_typedef.h"

OS_SEC_L2_TEXT U64 PRT_ClkGetCycleCount64(void)
{
    U64 cycle;
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    cycle = *(U64 *)CLINT_TIME;
    PRT_HwiRestore(intSave);
    return cycle;
}

