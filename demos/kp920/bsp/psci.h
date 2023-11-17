#ifndef __PSCI_H__
#define __PSCI_H__

extern U32 OsArmSmccSmc(U64 a0, U64 a1, U64 a2, U64 a3, U64 a4, U64 a5, U64 a6,
    U64 a7);
extern void OsCpuPowerOff(void);

#endif