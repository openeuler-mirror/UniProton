#ifndef _ARCH_INTERFACE_H_
#define _ARCH_INTERFACE_H_

#include "gdbstub_common.h"

/**
 * Architecture layer debug start
 */
extern void OsGdbArchInit(void);

/**
 *
 * Continue software execution.
 */
extern void OsGdbArchContinue(void);

/**
 *
 * Continue software execution until reaches the next statement.
 */
extern void OsGdbArchStep(void);

/**
 * Read all registers, and outputs as hexadecimal string.
 *
 * This reads all CPU registers and outputs as hexadecimal string.
 * The output string must be parsable by GDB.
 *
 * @param buf    Buffer to output hexadecimal string.
 * @param buflen Length of buffer.
 *
 * @return Length of hexadecimal string written.
 *         Return 0 if error or not supported.
 */

extern int OsGdbArchReadAllRegs(U8 *buf, int buflen);

/**
 * Take a hexadecimal string and update all registers.
 *
 * This takes in a hexadecimal string as presented from GDB,
 * and updates all CPU registers with new values.
 *
 * @param hex    Input hexadecimal string.
 * @param hexlen Length of hexadecimal string.
 *
 * @return Length of hexadecimal string parsed.
 *         Return 0 if error or not supported.
 */
extern int OsGdbArchWriteAllRegs(U8 *hex, int hexlen);

extern int OsGdbArchReadReg(U32 regno, U8 *buf, int buflen);

extern int OsGdbArchWriteReg(U32 regno, U8 *buf, int buflen);

extern int OsGdbArchRemoveSwBkpt(struct GdbBkpt *bkpt);

extern int OsGdbArchSetSwBkpt(struct GdbBkpt *bkpt);

extern void OsGdbArchPrepare(void *stk);

extern void OsGdbArchFinish(void *stk);

extern void OsGdbArchInit(void);

/**
 * Allow an architecture to specify how to correct the hardware debug registers.
 */
extern void OsGdbArchCorrectHwBkpts(void);

/**
 * Allow an architecture to specify how to remove a hardware breakpoint.
 */
extern int OsGdbArchRemoveHwBkpt(uintptr_t addr, int len, enum GdbBkptType bptype);

/**
 * Allow an architecture to specify how to set a hardware breakpoint.
 */
extern int OsGdbArchSetHwBkpt(uintptr_t addr, int len, enum GdbBkptType bptype);

/**
 * Allow an architecture to specify how to disable hardware breakpoints for a single cpu.
 */
extern void OsGdbArchDisableHwBkpts();

/**
 * Allow an architecture to specify how to remove all hardware breakpoints.
 */
extern void OsGdbArchRemoveAllHwBkpts(void);

extern int OsGdbArchHitHwBkpt(uintptr_t *addr, unsigned *type);

extern int OsGdbArchNotifyDie(int action, void *data);

extern int OsGdbGetStopReason();

#endif /* _ARCH_INTERFACE_H_ */