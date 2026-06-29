/*
 * Copyright (c) 2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: bestfit allocator compatibility layer for UniProton.
 */
#ifndef PRT_BESTFIT_COMPAT_H
#define PRT_BESTFIT_COMPAT_H

#include "prt_typedef.h"
#include "prt_buildef.h"
#include "prt_config.h"
#include "securec.h"
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef U8 UINT8;
typedef U16 UINT16;
typedef U32 UINT32;
typedef U64 UINT64;
typedef S8 INT8;
typedef S16 INT16;
typedef S32 INT32;
typedef S64 INT64;
typedef uintptr_t UINTPTR;
typedef intptr_t INTPTR;
typedef char CHAR;
typedef bool BOOL;

#ifndef VOID
#define VOID void
#endif

#ifndef STATIC
#define STATIC static
#endif

#ifdef INLINE
#undef INLINE
#endif
#define INLINE inline __attribute__((always_inline))

#ifndef STATIC_INLINE
#define STATIC_INLINE static inline
#endif

#ifndef PRT_OK
#define PRT_OK 0U
#endif

#ifndef PRT_NOK
#define PRT_NOK 1U
#endif

#ifndef OS_OK
#define OS_OK 0U
#endif

#ifndef OS_ERROR
#define OS_ERROR ((UINT32)-1)
#endif

#ifndef OS_NULL_INT
#define OS_NULL_INT ((UINT32)0xFFFFFFFF)
#endif

#ifndef OS_INVALID
#define OS_INVALID ((UINT32)-1)
#endif

#ifndef TRUE
#define TRUE ((BOOL)1)
#endif

#ifndef FALSE
#define FALSE ((BOOL)0)
#endif

#ifndef YES
#define YES 1
#endif

#ifndef NO
#define NO 0
#endif

#if defined(__aarch64__) && !defined(PRT_BESTFIT_AARCH64)
#define PRT_BESTFIT_AARCH64
#endif

#define PRT_BESTFIT_KERNEL_MEM

#if defined(OS_MEM_SLAB_EXTENTION)
#define PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_EXTENTION
#endif

#if defined(OS_MEM_SLAB_AUTO_EXPANSION_MODE)
#define PRT_BESTFIT_CFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
#endif

#define PRT_BESTFIT_SEC_ALW_INLINE
#define PRT_BESTFIT_SEC_TEXT
#define PRT_BESTFIT_SEC_TEXT_MINOR
#define PRT_BESTFIT_SEC_TEXT_INIT
#define PRT_BESTFIT_SEC_DATA
#define PRT_BESTFIT_SEC_DATA_INIT
#define PRT_BESTFIT_SEC_BSS
#define PRT_BESTFIT_SEC_BSS_MINOR
#define PRT_BESTFIT_SEC_BSS_INIT

#ifndef CLZ
#define CLZ __builtin_clz
#endif

#define PRT_HighBitGet(value) ((UINT32)(31U - (UINT32)CLZ(value)))
#ifndef OS_INT_INACTIVE
#define OS_INT_INACTIVE 1
#endif

#ifdef OS_TSK_MAX_SUPPORT_NUM
#define PRT_BESTFIT_BASE_CORE_TSK_LIMIT OS_TSK_MAX_SUPPORT_NUM
#else
#define PRT_BESTFIT_BASE_CORE_TSK_LIMIT 31
#endif

#ifndef OS_SYS_MEM_SIZE
#define OS_SYS_MEM_SIZE 0
#endif

#ifndef PRT_BESTFIT_RECORD_LR_CNT
#define PRT_BESTFIT_RECORD_LR_CNT 0
#endif

#ifndef PRT_BESTFIT_OMIT_LR_CNT
#define PRT_BESTFIT_OMIT_LR_CNT 0
#endif

#define PRT_BESTFIT_MEM_CHECK_LEVEL_LOW 0
#define PRT_BESTFIT_MEM_CHECK_LEVEL_HIGH 1
#define PRT_BESTFIT_MEM_CHECK_LEVEL_DISABLE 0xFF
#define PRT_BESTFIT_MEM_CHECK_LEVEL_DEFAULT PRT_BESTFIT_MEM_CHECK_LEVEL_DISABLE

#define PRT_BESTFIT_ERRNO_MEMCHECK_PARA_NULL 0x02000101U
#define PRT_BESTFIT_ERRNO_MEMCHECK_OUTSIDE 0x02000102U
#define PRT_BESTFIT_ERRNO_MEMCHECK_NO_HEAD 0x02000103U
#define PRT_BESTFIT_ERRNO_MEMCHECK_WRONG_LEVEL 0x02000104U
#define PRT_BESTFIT_ERRNO_MEMCHECK_DISABLED 0x02000105U

typedef VOID (*MALLOC_HOOK)(VOID);

extern unsigned int PRT_Printf(const char *format, ...);

#define PRINTK(fmt, ...)      (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_INFO(fmt, ...)  (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_ERR(fmt, ...)   (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_WARN(fmt, ...)  (void)PRT_Printf(fmt, ##__VA_ARGS__)

#define PRT_Panic(fmt, ...) do { \
    (void)PRT_Printf(fmt, ##__VA_ARGS__); \
    while (1) { } \
} while (0)

typedef struct {
    UINT32 dummy;
} SPIN_LOCK_S;

#define SPIN_LOCK_INIT(lock) SPIN_LOCK_S lock = { 0 }
#define PRT_SpinLockSave(lock, state) do { \
    (void)(lock); \
    *(state) = 0; \
} while (0)
#define PRT_SpinUnlockRestore(lock, state) do { \
    (void)(lock); \
    (void)(state); \
} while (0)
#define PRT_SpinLock(lock) do { (void)(lock); } while (0)
#define PRT_SpinUnlock(lock) do { (void)(lock); } while (0)

#define PRT_TRACE(event, ...)
#define MEM_INFO_REQ 0
#define MEM_ALLOC 1
#define MEM_ALLOC_ALIGN 2
#define MEM_FREE 3
#define MEM_REALLOC 4

#define OsBackTrace() do { } while (0)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_BESTFIT_COMPAT_H */
