#include "os_exc_riscv64g.h"
#include "prt_typedef.h"
#include "platform.h"

#define OS_IS_INTERUPT(mcause)     (mcause & 0x8000000000000000ull)
#define OS_IS_EXCEPTION(mcause)    (~(OS_IS_INTERUPT))
#define OS_IS_TICK_INT(mcause)     (mcause == 0x8000000000000007ull)
#define OS_IS_SOFT_INT(mcause)     (mcause == 0x8000000000000003ull)
#define OS_IS_EXT_INT(mcause)      (mcause == 0x800000000000000bull)
#define OS_IS_TRAP_USER(mcause)    (mcause == 0x000000000000000bull)
#define OsTskTrap_Magic             1579

extern void OsExcDispatch();
extern void hwi_handler();
extern void hwi_timer_handler();
extern void OsMainSchedule();

void clear_soft_pending()
{
    *(U32*)CLINT_MSI = 0;
}

void trap_entry(U64 mcause)
{
    if(OS_IS_INTERUPT(mcause)) {
        if(OS_IS_TICK_INT(mcause)) {
            hwi_timer_handler();
        } else if(OS_IS_SOFT_INT(mcause)) {
            clear_soft_pending();
        } else if(OS_IS_EXT_INT(mcause)) {
            hwi_handler();
        }
    } else {
        OsExcDispatch();
    }
    
}