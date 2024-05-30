#include "prt_typedef.h"
#include "prt_gdbstub_ext.h"

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

STUB_TEXT struct GdbRingBufferCfg *OsGetGdbRingBufferCfg()
{
    return &g_rbufCfg;
}