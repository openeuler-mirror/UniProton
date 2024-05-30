#ifndef _GDBSTUB_COMMON_H_
#define _GDBSTUB_COMMON_H_

#include "gdbstub.h"
#include "prt_typedef.h"
#include "prt_gdbstub_ext.h"

/* Map from CPU exceptions to GDB  */
#define GDB_EXCEPTION_INVALID_INSTRUCTION   4
#define GDB_EXCEPTION_BREAKPOINT            5
#define GDB_EXCEPTION_MEMORY_FAULT          7
#define GDB_EXCEPTION_DIVIDE_ERROR          8
#define GDB_EXCEPTION_INVALID_MEMORY        11
#define GDB_EXCEPTION_OVERFLOW              16

/* Access permissions for memory regions */

#define GDB_MEM_REGION_NO_ACCESS            0UL
#define GDB_MEM_REGION_READ                 BIT(0)
#define GDB_MEM_REGION_WRITE                BIT(1)


/* breakpoints */
#define GDB_MAX_BREAKPOINTS                 64

enum GdbBkptType {
    BP_BREAKPOINT = 0,
    BP_HARDWARE_BREAKPOINT,
    BP_WRITE_WATCHPOINT,
    BP_READ_WATCHPOINT,
    BP_ACCESS_WATCHPOINT,
    BP_POKE_BREAKPOINT,
};

enum GdbBkptState {
    BP_UNDEFINED = 0,
    BP_REMOVED,
    BP_SET,
    BP_ACTIVE
};

struct GdbBkpt {
    uintptr_t               addr;
    unsigned char           instr[BREAK_INSTR_SIZE];
    enum GdbBkptType        type;
    enum GdbBkptState       state;
};

/* regs */
struct DbgRegDef {
    char *name;
    int size;
    int offset;
};

static inline const char *GetWatchTypeStr(unsigned type)
{
    if (type < BP_WRITE_WATCHPOINT || type > BP_ACCESS_WATCHPOINT) {
        return NULL;
    }
    if (type == BP_WRITE_WATCHPOINT) {
        return "watch";
    } else if (type == BP_ACCESS_WATCHPOINT) {
        return "awatch";
    } else {
        return "rwatch";
    }
}

extern STUB_TEXT void OsGdbHandleException(void *stk);
#endif /* _GDBSTUB_COMMON_H_ */
