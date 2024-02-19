
#include <pthread.h>
#include <assert.h>
#include <errno.h>

#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_hwi.h"

#if defined(CONFIG_FS_LARGEFILE)
#define PRIdOFF PRId64
#define PRIuOFF PRIu64
#else
#define PRIdOFF PRId32
#define PRIuOFF PRIu32
#endif

#define DEBUGVERIFY(f) ((void)(f))
#define DEBUGASSERT(f) assert(f)

#define PANIC() assert("panic")

#define OK (0)
#define ERROR (-1)

#define set_errno(e) \
    do \
        { \
            errno = (int)(e); \
        } \
    while (0)
#define get_errno() errno

typedef void* pthread_startroutine_t;

/**
 * From Nuttx statfs.h
 **/
#define PROC_SUPER_MAGIC      0x9fa0

/**
 *  From Nuttx fcntl.h
 **/
#define NUTTX_O_ACCMODE 03

/**
 *  From Nuttx libc.h
 **/
#ifndef CONFIG_LIBC_HOMEDIR
#define CONFIG_LIBC_HOMEDIR "/"
#endif

/**
 * 仅支持单核, 该功能不支持
 **/
#ifdef CONFIG_SMP
#undef CONFIG_SMP
#endif

#ifdef CONFIG_IRQCOUNT
#undef CONFIG_IRQCOUNT
#endif

/**
 * 不支持该机制
 **/
#ifdef CONFIG_PM
#undef CONFIG_PM
#endif

/**
 * 以下宏定义暂不支持相应功能
 **/
#ifdef CONFIG_FS_AIO
#undef CONFIG_FS_AIO
#endif

#ifdef CONFIG_FS_RPMSGFS_SERVER
#undef CONFIG_FS_RPMSGFS_SERVER
#endif

#ifdef CONFIG_FDSAN
#undef CONFIG_FDSAN
#endif

#ifdef CONFIG_FDCHECK
#undef CONFIG_FDCHECK
#endif

#ifdef CONFIG_FDCLONE_STDIO
#undef CONFIG_FDCLONE_STDIO
#endif

#ifdef CONFIG_CANCELLATION_POINTS
#undef CONFIG_CANCELLATION_POINTS
#endif

#ifndef CONFIG_DISABLE_ENVIRON
#define CONFIG_DISABLE_ENVIRON
#endif

/**
 * 以下类型FS暂不支持
 **/
#ifdef CONFIG_FS_ROMFS
#undef CONFIG_FS_ROMFS
#endif

#ifdef CONFIG_FS_SMARTFS
#undef CONFIG_FS_SMARTFS
#endif

#ifdef CONFIG_FS_LITTLEFS
#undef CONFIG_FS_LITTLEFS
#endif

#ifdef CONFIG_FS_SPIFFS
#undef CONFIG_FS_SPIFFS
#endif

#ifdef CONFIG_FS_LITTLEFS
#undef CONFIG_FS_LITTLEFS
#endif
