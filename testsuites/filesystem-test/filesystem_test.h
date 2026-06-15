#ifndef FILESYSTEM_TEST_H
#define FILESYSTEM_TEST_H

#include "nuttx/config.h"

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_DRIVER) && defined(CONFIG_FS_FAT) && defined(CONFIG_FILE_STREAM)
#define FS_ENABLE_FATFS_TEST 1
#endif

#if defined(OS_OPTION_NUTTX_VFS) && defined(CONFIG_FS_RAMFS) && defined(CONFIG_FILE_STREAM)
#define FS_ENABLE_RAMFS_TEST 1
#endif

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_DRIVER) && defined(CONFIG_MTD) && \
    defined(CONFIG_MTD_BYTE_WRITE) && defined(CONFIG_FS_SPIFFS) && defined(CONFIG_FILE_STREAM)
#define FS_ENABLE_SPIFFS_TEST 1
#endif

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_DRIVER) && defined(CONFIG_MTD) && \
    defined(CONFIG_MTD_BYTE_WRITE) && defined(CONFIG_FS_LITTLEFS)
#define FS_ENABLE_LITTLEFS_TEST 1
#endif

#if defined(OS_OPTION_NUTTX_VFS) && defined(OS_OPTION_DRIVER)
#define FS_ENABLE_DEVFS_TEST 1
#endif

#if defined(FS_ENABLE_FATFS_TEST) || defined(FS_ENABLE_RAMFS_TEST) || defined(FS_ENABLE_SPIFFS_TEST) || \
    defined(FS_ENABLE_LITTLEFS_TEST) || defined(FS_ENABLE_DEVFS_TEST)
#define FS_HAS_ENABLED_TEST 1
#endif

int FatfsTest(void);
int RamfsTest(void);
int SpiffsTest(void);
int LittlefsTest(void);
int DevfsTest(void);

#endif
