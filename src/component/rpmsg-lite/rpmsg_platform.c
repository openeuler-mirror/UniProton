/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rpmsg_platform.h"
#include "rpmsg_env.h"
#include "rpmsg_compiler.h"

#if defined(RL_USE_ENVIRONMENT_CONTEXT) && (RL_USE_ENVIRONMENT_CONTEXT == 1)
#error "This RPMsg-Lite port requires RL_USE_ENVIRONMENT_CONTEXT set to 0"
#endif

static int32_t disable_counter;
static void *platform_lock;
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
static LOCK_STATIC_CONTEXT platform_lock_static_ctxt;
#endif

static void platform_global_isr_disable(void) {}
static void platform_global_isr_enable(void) {}

int32_t RL_WEAK platform_init_interrupt(uint32_t vector_id, void *isr_data)
{
    platform_global_isr_disable();
    platform_global_isr_enable();
    return 0;
}

int32_t RL_WEAK platform_deinit_interrupt(uint32_t vector_id)
{
    platform_global_isr_disable();
    platform_global_isr_enable();
    return 0;
}

void RL_WEAK platform_notify(uint32_t vector_id) {}

/**
 * platform_time_delay
 *
 * @param num_msec Delay time in ms.
 *
 * This is not an accurate delay, it ensures at least num_msec passed when return.
 */
void RL_WEAK platform_time_delay(uint32_t num_msec) {}

/**
 * platform_interrupt_enable
 *
 * Enable peripheral-related interrupt
 *
 * @param vector_id Virtual vector ID that needs to be converted to IRQ number
 *
 * @return vector_id Return value is never checked.
 *
 */
int32_t RL_WEAK platform_interrupt_enable(uint32_t vector_id)
{
    return 0;
}

/**
 * platform_interrupt_disable
 *
 * Disable peripheral-related interrupt.
 *
 * @param vector_id Virtual vector ID that needs to be converted to IRQ number
 *
 * @return vector_id Return value is never checked.
 *
 */
int32_t RL_WEAK platform_interrupt_disable(uint32_t vector_id)
{
    return 0;
}

/**
 * platform_map_mem_region
 *
 * Dummy implementation
 *
 */
void RL_WEAK platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr, uint32_t size, uint32_t flags)
{
}

/**
 * platform_cache_all_flush_invalidate
 *
 * Dummy implementation
 *
 */
void RL_WEAK platform_cache_all_flush_invalidate(void)
{
}

/**
 * platform_cache_disable
 *
 * Dummy implementation
 *
 */
void RL_WEAK platform_cache_disable(void)
{
}

/**
 * platform_vatopa
 *
 * Dummy implementation
 *
 */
uintptr_t RL_WEAK platform_vatopa(void *addr)
{
    return (uintptr_t)0;
}

/**
 * platform_patova
 *
 * Dummy implementation
 *
 */
void * RL_WEAK platform_patova(uintptr_t addr)
{
    return ((void *)0);
}

/**
 * platform_init
 *
 * platform/environment init
 */
int32_t RL_WEAK platform_init(void)
{
    platform_lock = NULL;
    disable_counter = 0;
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    platform_lock_static_ctxt = 0;
#endif
    return 0;
}

/**
 * platform_deinit
 *
 * platform/environment deinit process
 */
int32_t RL_WEAK platform_deinit(void)
{
    return 0;
}

#if defined(RL_ALLOW_CUSTOM_SHMEM_CONFIG) && (RL_ALLOW_CUSTOM_SHMEM_CONFIG == 1)
/**
 * platform_get_custom_shmem_config
 *
 * Provide rpmsg_platform_shmem_config structure for the rpmsg_lite instance initialization
 * based on the link_id.
 *
 * @param link_id Link ID provided by the rpmsg_lite init function
 * @param cfg Pointer to the rpmsg_platform_shmem_config structure to be filled
 *
 * @return Status of function execution, 0 on success.
 *
 */
int32_t RL_WEAK platform_get_custom_shmem_config(uint32_t link_id, rpmsg_platform_shmem_config_t *cfg)
{
    return 0;
}

#endif /* defined(RL_ALLOW_CUSTOM_SHMEM_CONFIG) && (RL_ALLOW_CUSTOM_SHMEM_CONFIG == 1) */
