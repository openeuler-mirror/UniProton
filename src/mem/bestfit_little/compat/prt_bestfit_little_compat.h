/*
 * Copyright (c) 2026 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * Description: bestfit_little allocator compatibility layer for UniProton.
 */
#ifndef PRT_BESTFIT_LITTLE_COMPAT_H
#define PRT_BESTFIT_LITTLE_COMPAT_H

#include "prt_typedef.h"
#include "prt_buildef.h"
#include <stdint.h>

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

#ifndef LOS_OK
#define LOS_OK 0U
#endif

#ifndef LOS_NOK
#define LOS_NOK 1U
#endif

#ifndef OS_NULL_INT
#define OS_NULL_INT ((UINT32)0xFFFFFFFF)
#endif

#ifndef OS_OK
#define OS_OK 0U
#endif

#ifndef OS_ERROR
#define OS_ERROR ((UINT32)-1)
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

#define PRT_BESTFIT_LITTLE_SEC_ALW_INLINE
#define PRT_BESTFIT_LITTLE_SEC_TEXT
#define PRT_BESTFIT_LITTLE_SEC_TEXT_MINOR
#define PRT_BESTFIT_LITTLE_SEC_TEXT_INIT
#define PRT_BESTFIT_LITTLE_SEC_DATA
#define PRT_BESTFIT_LITTLE_SEC_DATA_INIT
#define PRT_BESTFIT_LITTLE_SEC_BSS
#define PRT_BESTFIT_LITTLE_SEC_BSS_MINOR
#define PRT_BESTFIT_LITTLE_SEC_BSS_INIT

#ifndef CLZ
#define CLZ __builtin_clz
#endif

#ifndef UNUSED
#define UNUSED(x) ((void)(x))
#endif

extern unsigned int PRT_Printf(const char *format, ...);

#define PRINTK(fmt, ...)      (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_INFO(fmt, ...)  (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_ERR(fmt, ...)   (void)PRT_Printf(fmt, ##__VA_ARGS__)
#define PRINT_WARN(fmt, ...)  (void)PRT_Printf(fmt, ##__VA_ARGS__)

#define LOS_Panic(fmt, ...) do { \
    (void)PRT_Printf(fmt, ##__VA_ARGS__); \
    while (1) { } \
} while (0)

typedef struct {
    UINT32 dummy;
} SPIN_LOCK_S;

#define SPIN_LOCK_INIT(lock) SPIN_LOCK_S lock = { 0 }
#define LOS_SpinLockSave(lock, state) do { \
    (void)(lock); \
    *(state) = 0; \
} while (0)
#define LOS_SpinUnlockRestore(lock, state) do { \
    (void)(lock); \
    (void)(state); \
} while (0)

#define LOS_TRACE(event, ...)
#define MEM_INFO_REQ 0
#define MEM_ALLOC 1
#define MEM_ALLOC_ALIGN 2
#define MEM_FREE 3
#define MEM_REALLOC 4

#if defined(OS_MEM_SLAB_EXTENTION)
#define LOSCFG_KERNEL_MEM_SLAB_EXTENTION
#endif

#if defined(OS_MEM_SLAB_AUTO_EXPANSION_MODE)
#define LOSCFG_KERNEL_MEM_SLAB_AUTO_EXPANSION_MODE
#endif

/*
 * Slab calls OsMemAlloc/OsMemFree as its backing heap hooks. UniProton already
 * owns public OsMemAlloc/OsMemAllocAlign symbols, so keep the slab backing
 * hooks local to bestfit_little by renaming them at preprocessing time.
 */
#ifdef LOSCFG_KERNEL_MEM_SLAB_EXTENTION
extern VOID *OsPrtHeapAllocForSlab(VOID *pool, UINT32 size);
extern UINT32 OsPrtHeapFreeForSlab(VOID *pool, VOID *mem);
#define OsMemAlloc OsPrtHeapAllocForSlab
#define OsMemFree OsPrtHeapFreeForSlab
#endif

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* PRT_BESTFIT_LITTLE_COMPAT_H */
