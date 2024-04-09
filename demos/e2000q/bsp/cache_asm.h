#ifndef __CACHE_H__
#define __CACHE_H__

#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_module.h"
#include "prt_errno.h"

extern void os_asm_invalidate_dcache_all(void);
extern void os_asm_invalidate_icache_all(void);
extern void os_asm_invalidate_tlb_all(void);
#endif
