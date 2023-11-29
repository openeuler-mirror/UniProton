#include "prt_typedef.h"
#include "prt_gdbstub_ext.h"

extern const char __os_print_start[];
extern const char __os_sys_sp_end[];

extern const char __os_stub_data_start[];
extern const char __os_stub_data_end[];
extern const char __os_stub_text_start[];
extern const char __os_stub_text_end[];

static STUB_DATA struct GdbMemRegion g_regions[] = {
    {
        .start = (uintptr_t)__os_print_start,
        .end = (uintptr_t)__os_sys_sp_end,
        .attributes = GDB_MEM_REGION_RW
    },
    {
        .start = (uintptr_t)__os_stub_text_start,
        .end = (uintptr_t)__os_stub_text_end,
        .attributes = GDB_MEM_REGION_NO_SWBKPT
    },
    {
        .start = (uintptr_t)__os_stub_data_start,
        .end = (uintptr_t)__os_stub_data_end,
        .attributes = GDB_MEM_REGION_NO_SWBKPT
    }
};

/*
    reserved for log
    .va = 0xf02400000,
    .pa = 0x0,
    .size = 0x200000,
    .attr = MEM_ATTR_UNCACHE_RWX,
    .page_size = PAGE_SIZE_2M,
*/
static STUB_DATA struct GdbRingBufferCfg g_rbufCfg = {
    .rxaddr = 0xf02600000 - 0x3000,
    .txaddr = 0xf02600000 - 0x4000,
    .size = 0x1000
};

STUB_TEXT int OsGdbConfigGetMemRegions(struct GdbMemRegion **regions)
{
    if (!regions) {
        return 0;
    }
    *regions = g_regions;
    return sizeof(g_regions) / sizeof(struct GdbMemRegion);
}

STUB_TEXT struct GdbRingBufferCfg *OsGetGdbRingBufferCfg()
{
    return &g_rbufCfg;
}