#include "riscv_ipi_external.h"
#include "prt_hwi.h"
#include "prt_buildef.h"
#include "platform.h"
#include "shmem_self.h"
#include "uart.h"
#define IPI_MAGIC 0x12369

static ipi_handler ipi_hook_in;
static int ipi_state;

U32 riscv_ipi_handler(U32 ipidata)
{
	if(ipi_state != IPI_MAGIC) {
		return  OS_ERRNO_IPI_NOHANDLER;
	}
	if(ipi_hook_in != NULL) {
		return ipi_hook_in(ipidata);
	}
	return OS_OK;
}

U32 register_ipi_handler(ipi_handler ipi_hook)
{
	if(ipi_hook == NULL) {
		return OS_ERRNO_IPI_HOOKERROR;
	}
	uintptr_t int_save = PRT_HwiLock();
	ipi_hook_in = ipi_hook;
	ipi_state = IPI_MAGIC;
	PRT_HwiRestore(int_save);
	return OS_OK;
}

U32 send_ipi(U32 target_core, U32 ipidata)
{
	/*
	if(core >= OS_MAX_CORE_NUM) {
		return OS_ERRNO_IPI_MAXCORE;
	}
	*/
	
	PUSH_SHMEM(ipidata, target_core);
	*(volatile U32*)CLINT_MSI_REG(target_core) = 1;
	return OS_OK;
}

