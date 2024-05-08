#ifndef PRT_AMP_PSCI_INTERNAL_H
#define PRT_AMP_PSCI_INTERNAL_H
#include "prt_hwi_external.h"
#include "prt_task_external.h"

#define PSCI_FN_BASE    0x84000000
#define PSCI_FN(val)    (PSCI_FN_BASE + (val))
#define PSCI_64BIT      0x40000000
#define PSCI_FN64_BASE  (PSCI_FN_BASE + PSCI_64BIT)
#define PSCI_FN64(n)    (PSCI_FN64_BASE + (n))

#define PSCI_FN_CPU_OFF PSCI_FN(2)
#define PSCI_FN_CPU_ON PSCI_FN64(3)
#define PSCI_FN_AFFINITY_INFO PSCI_FN64(4)

extern U32 OsArmSmccSmc(U64 a0, U64 a1, U64 a2, U64 a3, U64 a4, U64 a5, U64 a6,
    U64 a7);
extern void OsCpuPowerOff(void);

#endif