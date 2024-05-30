#ifndef _PRT_GDBSTUB_EXT_
#define _PRT_GDBSTUB_EXT_

#include "prt_typedef.h"

/** Describe one memory region */
struct GdbMemRegion {
    /** Start address of a memory region */
    uintptr_t start;

    /** End address of a memory region */
    uintptr_t end;

    /** Memory region attributes */
    uint16_t  attributes;
};

struct GdbRingBufferCfg {
    uintptr_t rxaddr;
    uintptr_t txaddr;
    int  size;
};

#define STUB_TEXT __attribute__((__section__(".stub.text")))
#define STUB_DATA __attribute__((__section__(".stub.data")))

#ifndef __weak
#define __weak __attribute__((__weak__))
#endif

#ifndef BIT
#define BIT(n)  (1UL << (n))
#endif

/* Access permissions for memory regions */
#define GDB_MEM_REGION_NO_ACCESS            0UL
#define GDB_MEM_REGION_READ                 BIT(0)
#define GDB_MEM_REGION_WRITE                BIT(1)
#define GDB_MEM_REGION_NO_BKPT              BIT(2)

#define GDB_MEM_REGION_RO \
    (GDB_MEM_REGION_READ)

#define GDB_MEM_REGION_RW \
    (GDB_MEM_REGION_READ | GDB_MEM_REGION_WRITE)

#endif /* _PRT_GDBSTUB_EXT_ */
