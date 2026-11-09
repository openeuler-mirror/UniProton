/* SPDX-License-Identifier: MulanPSL-2.0 */
#ifndef MICA_UNIPROTON_MEMORY_H
#define MICA_UNIPROTON_MEMORY_H
#include <stddef.h>
#include <stdint.h>
#include <errno.h>
#include "prt_mem.h"

/* aarch64_xen BSP supplies and maintains an identity local GPA view, M0C0 and
 * shared-page attributes before entering MICA. This port does not establish or
 * test that mode. Caller supplies valid outputs and nonzero power-of-two align. */
static inline int mica_mem_alloc(size_t bytes, size_t align, void **base)
{
    *base = NULL;
    if (bytes > UINT32_MAX)
        return -EOVERFLOW;

    enum MemAlign native_align = (enum MemAlign)__builtin_ctzl(align);

    *base = PRT_MemAllocAlign(OS_MID_SYS, OS_MEM_DEFAULT_FSC_PT, (U32)bytes, native_align);
    return *base ? 0 : -ENOMEM;
}

static inline int mica_mem_free(void *base)
{
    return PRT_MemFree(OS_MID_SYS, base) == OS_OK ? 0 : -EIO;
}

static inline int mica_mem_address(const void *local, uint64_t *physical)
{
    *physical = (uint64_t)(uintptr_t)local;
    return 0;
}

static inline int mica_mem_map(uint64_t physical, size_t bytes, void **local)
{
    (void)bytes;
    *local = (void *)(uintptr_t)physical;
    return 0;
}

static inline int mica_mem_unmap(void *local, size_t bytes)
{
    (void)local;
    (void)bytes;
    /* Borrowed identity view: never free foreign or allocator storage here. */
    return 0;
}
#endif
