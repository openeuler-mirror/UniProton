#ifndef __RISCV_IPI_H
#define __RISCV_IPI_H

#include "prt_typedef.h"
#include "prt_errno.h"
#include "prt_module.h"

//because ipi is also a interrupt so we use OS_MID_HWI module
#define OS_ERRNO_IPI_NOHANDLER OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0100)

#define OS_ERRNO_IPI_HOOKERROR OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0101)

#define OS_ERRNO_IPI_MAXCORE OS_ERRNO_BUILD_ERROR(OS_MID_HWI, 0x0102)

typedef U32 (*ipi_handler)(U32 ipidata);

U32 send_ipi(U32 target_core, U32 ipidata);

U32 register_ipi_handler(ipi_handler ipi_hook);


#endif
