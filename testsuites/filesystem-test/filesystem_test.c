#include "filesystem_test.h"

extern unsigned int PRT_Printf(const char *format, ...);

#ifdef OS_OPTION_NUTTX_VFS
extern void fs_initialize(void);
#endif

void filesystem_test(void)
{
    int ret;

    PRT_Printf("[FS][INFO] start sd3403 filesystem tests\n");
#ifndef FS_HAS_ENABLED_TEST
    PRT_Printf("[FS][ERROR] no filesystem tests enabled\n");
    return;
#endif

#ifdef OS_OPTION_NUTTX_VFS
    fs_initialize();
#endif

    ret = FatfsTest();
    ret |= RamfsTest();
    ret |= SpiffsTest();
    ret |= LittlefsTest();
    ret |= DevfsTest();
    if (ret == 0) {
        PRT_Printf("[FS][INFO] sd3403 filesystem tests success\n");
    } else {
        PRT_Printf("[FS][ERROR] sd3403 filesystem tests failed\n");
    }
}
