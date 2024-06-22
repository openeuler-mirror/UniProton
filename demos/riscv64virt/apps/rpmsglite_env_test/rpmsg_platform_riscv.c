/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rpmsg_platform.h"
#include "rpmsg_env.h"
#include "rpmsg_compiler.h"
#include "riscv.h"
#include "riscv_ipi.h"
#include "os_cpu_riscv64.h"
#include "prt_tick.h"
#include "prt_config.h"
#include "prt_task.h"

#if defined(RL_USE_ENVIRONMENT_CONTEXT) && (RL_USE_ENVIRONMENT_CONTEXT == 1)
#error "This RPMsg-Lite port requires RL_USE_ENVIRONMENT_CONTEXT set to 0"
#endif

static volatile uintptr_t g_intsave = 0;
static uint32_t g_cur_remotecore = 0;
static int32_t disable_counter = 0;
static void *platform_lock;
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
static LOCK_STATIC_CONTEXT platform_lock_static_ctxt;
#endif

static void platform_global_isr_disable(void)
{
    uintptr_t mstatus_r;
    uintptr_t mstatus_w;
    OS_EMBED_ASM("csrr %0,mstatus":"=r"(mstatus_r)::);
    mstatus_w = mstatus_r & (~MIE_S);
    OS_EMBED_ASM("csrw mstatus,%0"::"r"(mstatus_w):);
    g_intsave = (mstatus_r & MIE);
}

static void platform_global_isr_enable(void)
{
    if(g_intsave & MIE ) {
        uintptr_t mstatus_r;
        uintptr_t mstatus_w;
        OS_EMBED_ASM("csrr %0,mstatus":"=r"(mstatus_r)::);
        mstatus_w = mstatus_r | MIE_S;
        OS_EMBED_ASM("csrw mstatus,%0"::"r"(mstatus_w):);
        g_intsave = (mstatus_r & MIE);
    } else {
        platform_global_isr_disable();
    }
    return;
}

int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data)
{
    env_lock_mutex(platform_lock);
    /* Register ISR to environment layer */
    env_register_isr(vector_id, isr_data);
    env_unlock_mutex(platform_lock);

    return 0;
}

int32_t platform_deinit_interrupt(uint32_t vector_id)
{
    env_lock_mutex(platform_lock);
    /* Unregister ISR from environment layer */
    env_unregister_isr(vector_id);
    env_unlock_mutex(platform_lock);

    return 0;
}

U32 rpmsg_inner_rtos_ipi_handler(U32 ipidata)
{
    uint32_t target = ipidata;
    env_isr(target);
    return 0;
}

void platform_notify(uint32_t vector_id)
{
    env_lock_mutex(platform_lock);
    send_ipi(g_cur_remotecore, vector_id);
    env_unlock_mutex(platform_lock);
}

/**
 * platform_time_delay
 *
 * @param num_msec Delay time in ms.
 *
 * This is not an accurate delay, it ensures at least num_msec passed when return.
 */
#define MSEC_PER_SEC 1000
extern struct TickModInfo g_tickModInfo;
void platform_time_delay(uint32_t num_msec)
{
    uint64_t ticks = OS_SYS_CLOCK / g_tickModInfo.tickPerSecond;
    if (ticks < MSEC_PER_SEC)
        env_print("ERROR: No specific sys clock frequency or not enough ticks per second!\n");
    ticks = num_msec * ticks / MSEC_PER_SEC;
    /*delay the task for @ticks*/
    PRT_TaskDelay(ticks);
}

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
int32_t platform_interrupt_enable(uint32_t vector_id)
{
    RL_ASSERT(0 < disable_counter);

    platform_global_isr_disable();
    disable_counter--;

    if (disable_counter == 0)
    {
    }
    platform_global_isr_enable();

    return ((int32_t)vector_id);
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
int32_t platform_interrupt_disable(uint32_t vector_id)
{
    RL_ASSERT(0 <= disable_counter);

    platform_global_isr_disable();
    /* virtqueues use the same GIC vector
       if counter is set - the interrupts are disabled */
    if (disable_counter == 0)
    {
    }
    disable_counter++;
    platform_global_isr_enable();

    return ((int32_t)vector_id);
}

/**
 * platform_map_mem_region
 *
 * Dummy implementation
 *
 */
void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr, uint32_t size, uint32_t flags)
{
}

/**
 * platform_cache_all_flush_invalidate
 *
 * Dummy implementation
 *
 */
void platform_cache_all_flush_invalidate(void)
{
}

/**
 * platform_cache_disable
 *
 * Dummy implementation
 *
 */
void platform_cache_disable(void)
{
}

/**
 * platform_vatopa
 *
 * Dummy implementation
 *
 */
uintptr_t platform_vatopa(void *addr)
{
    return ((uintptr_t)(char *)addr);
}

/**
 * platform_patova
 *
 * Dummy implementation
 *
 */
void *platform_patova(uintptr_t addr)
{
    return ((void *)(char *)addr);
}

/**
 * platform_init
 *
 * platform/environment init
 */
int32_t platform_init(void)
{

    /* Create lock used in multi-instanced RPMsg */
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    if (0 != env_create_mutex(&platform_lock, 1, &platform_lock_static_ctxt))
#else
    if (0 != env_create_mutex(&platform_lock, 1))
#endif
        return -1;
    if (register_ipi_handler(rpmsg_inner_rtos_ipi_handler) != 0) {
        env_delete_mutex(&platform_lock);
        return -1;
    }
    g_cur_remotecore = PRT_GetCoreID();
    g_cur_remotecore = (~g_cur_remotecore) & 1U;
    env_print("Platform init, cur remote core:%u\n", g_cur_remotecore);
    return 0;
}

/**
 * platform_deinit
 *
 * platform/environment deinit process
 */
int32_t platform_deinit(void)
{
    /* Delete lock used in multi-instanced RPMsg */
    env_delete_mutex(platform_lock);
    platform_lock = ((void *)0);

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
int32_t platform_get_custom_shmem_config(uint32_t link_id, rpmsg_platform_shmem_config_t *cfg)
{
    cfg->buffer_payload_size = RL_BUFFER_PAYLOAD_SIZE(link_id);
    cfg->buffer_count        = RL_BUFFER_COUNT(link_id);

    switch (link_id)
    {
        case UNI2UNI_RPMSGLITE_TEST_LINKID:
            cfg->vring_size  = VRING_SIZE;
            cfg->vring_align = VRING_ALIGN;
            break;
        default:
            /* All the cases have been listed above, the default clause should not be reached. */
            break;
    }
    return 0;
}

#endif /* defined(RL_ALLOW_CUSTOM_SHMEM_CONFIG) && (RL_ALLOW_CUSTOM_SHMEM_CONFIG == 1) */
