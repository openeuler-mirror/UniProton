/*
 * Copyright (c) 2014, Mentor Graphics Corporation
 * Copyright (c) 2015 Xilinx, Inc.
 * Copyright (c) 2016 Freescale Semiconductor, Inc.
 * Copyright 2016-2022 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/**************************************************************************
 * FILE NAME
 *
 *       rpmsg_env_uniproton.c
 *
 *
 * DESCRIPTION
 *
 *       This file is Uniproton OS Implementation of env layer for OpenAMP.
 *
 *
 **************************************************************************/

#include "rpmsg_compiler.h"
#include "rpmsg_env.h"
#include "rpmsg_platform.h"
#include "rpmsg_lite.h"
#include "virtqueue.h"

#include <stdlib.h>
#include <string.h>

#include "prt_sem.h"
#include "prt_queue.h"

static int32_t env_init_counter = 0;

/* Max supported ISR counts */
#define ISR_COUNT (12U)
/*!
 * Structure to keep track of registered ISR's.
 */
struct isr_info
{
    void *data;
};
static struct isr_info isr_table[ISR_COUNT];

#define RL_ENV_MAX_MUTEX_COUNT (8)

#if defined(RL_USE_ENVIRONMENT_CONTEXT) && (RL_USE_ENVIRONMENT_CONTEXT == 1)
#error "This RPMsg-Lite port requires RL_USE_ENVIRONMENT_CONTEXT set to 0"
#endif

/*!
 * env_wait_for_link_up
 *
 * Wait until the link_state parameter of the rpmsg_lite_instance is set.
 * Busy loop implementation for BM, timeout_ms parameter ignored for now.
 *
 */
uint32_t env_wait_for_link_up(volatile uint32_t *link_state, uint32_t link_id, uint32_t timeout_ms)
{
    while (*link_state != 1U)
    {
        env_sleep_msec(10);
    }
    return 1U;
}

/*!
 * env_tx_callback
 *
 * Set event to notify task waiting in env_wait_for_link_up().
 * Empty implementation for BM.
 *
 */
void env_tx_callback(uint32_t link_id)
{
}

/*!
 * env_init
 *
 * Initializes OS/BM environment.
 *
 */
int32_t env_init(void)
{
    // verify 'env_init_counter'
    RL_ASSERT(env_init_counter >= 0);
    if (env_init_counter < 0)
    {
        /* coco begin validated: (env_init_counter < 0) condition will never met unless RAM is corrupted */
        return -1;
        /* coco end */
    }
    env_init_counter++;
    // multiple call of 'env_init' - return ok
    if (1 < env_init_counter)
    {
        return 0;
    }
    (void)env_memset(isr_table, 0, sizeof(isr_table));
    return platform_init();
}

/*!
 * env_deinit
 *
 * Uninitializes OS/BM environment.
 *
 * @returns Execution status
 */
int32_t env_deinit(void)
{
    // verify 'env_init_counter'
    RL_ASSERT(env_init_counter > 0);
    if (env_init_counter <= 0)
    {
        return -1;
    }
    // counter on zero - call platform deinit
    env_init_counter--;
    // multiple call of 'env_deinit' - return ok
    if (0 < env_init_counter)
    {
        return 0;
    }
    // last call
    return platform_deinit();
}

/*!
 * env_allocate_memory - implementation
 *
 * @param size
 */
void *env_allocate_memory(uint32_t size)
{
    return (malloc(size));
}

/*!
 * env_free_memory - implementation
 *
 * @param ptr
 */
void env_free_memory(void *ptr)
{
    if (ptr != ((void *)0))
    {
        free(ptr);
    }
}
#define CHECK_FREE(ptr)       \
    while ((ptr) != NULL)     \
    {                         \
        env_free_memory(ptr); \
        (ptr) = NULL;         \
    }
/*!
 *
 * env_memset - implementation
 *
 * @param ptr
 * @param value
 * @param size
 */
void env_memset(void *ptr, int32_t value, uint32_t size)
{
    (void)memset(ptr, value, size);
}

/*!
 *
 * env_memcpy - implementation
 *
 * @param dst
 * @param src
 * @param len
 */
void env_memcpy(void *dst, void const *src, uint32_t len)
{
    (void)memcpy(dst, src, len);
}

/*!
 *
 * env_strcmp - implementation
 *
 * @param dst
 * @param src
 */

int32_t env_strcmp(const char *dst, const char *src)
{
    return (strcmp(dst, src));
}

/*!
 *
 * env_strncpy - implementation
 *
 * @param dest
 * @param src
 * @param len
 */
void env_strncpy(char *dest, const char *src, uint32_t len)
{
    (void)strncpy(dest, src, len);
}

/*!
 *
 * env_strncmp - implementation
 *
 * @param dest
 * @param src
 * @param len
 */
int32_t env_strncmp(char *dest, const char *src, uint32_t len)
{
    return (strncmp(dest, src, len));
}

/*!
 *
 * env_mb - implementation
 *
 */
void env_mb(void)
{
    MEM_BARRIER();
}

/*!
 * env_rmb - implementation
 */
void env_rmb(void)
{
    RMEM_BARRIER();
}

/*!
 * env_wmb - implementation
 */
void env_wmb(void)
{
    WMEM_BARRIER();
}

/*!
 * env_map_vatopa - implementation
 *
 * @param address
 */
uint32_t env_map_vatopa(void *address)
{
    return platform_vatopa(address);
}

/*!
 * env_map_patova - implementation
 *
 * @param address
 */
void *env_map_patova(uint32_t address)
{
    return platform_patova(address);
}

/*!
 * env_create_mutex
 *
 * Creates a mutex with the given initial count.
 *
 */
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
int32_t env_create_mutex(void **lock, int32_t count, void *context)
#else
int32_t env_create_mutex(void **lock, int32_t count)
#endif
{
    if (lock == NULL || count > RL_ENV_MAX_MUTEX_COUNT) {
        return 1;
    }
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    /* make the mutex pointer point to itself
     * this marks the mutex handle as initialized.
     */
    *lock = context;
#else
    if ((*lock = env_allocate_memory(sizeof(SemHandle))) == NULL) {
        return 1;
    }
#endif

    if (PRT_SemCreate(count, *((SemHandle **)lock)) != OS_OK) {
        CHECK_FREE(*lock);
        return 1;
    }

    return RL_SUCCESS;
}

/*!
 * env_delete_mutex
 *
 * Deletes the given lock
 *
 */
void env_delete_mutex(void *lock)
{
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    return;
#else
    PRT_SemDelete(*((SemHandle *)lock));
#endif
}

/*!
 * env_lock_mutex
 *
 * Tries to acquire the lock, if lock is not available then call to
 * this function will suspend.
 */
void env_lock_mutex(void *lock)
{
    if (lock != NULL)
    {
        PRT_SemPend(*((SemHandle *)lock), OS_WAIT_FOREVER);
    }
}

/*!
 * env_unlock_mutex
 *
 * Releases the given lock.
 */
void env_unlock_mutex(void *lock)
{
    if (lock != NULL)
    {
        PRT_SemPost(*((SemHandle *)lock));
    }
}

/*!
 * env_sleep_msec
 *
 * Suspends the calling thread for given time , in msecs.
 */
void env_sleep_msec(uint32_t num_msec)
{
    platform_time_delay(num_msec);
}

/*!
 * env_register_isr
 *
 * Registers interrupt handler data for the given interrupt vector.
 *
 * @param vector_id - virtual interrupt vector number
 * @param data      - interrupt handler data (virtqueue)
 */
void env_register_isr(uint32_t vector_id, void *data)
{
    RL_ASSERT(vector_id < ISR_COUNT);
    if (vector_id < ISR_COUNT)
    {
        isr_table[vector_id].data = data;
    }
}

/*!
 * env_unregister_isr
 *
 * Unregisters interrupt handler data for the given interrupt vector.
 *
 * @param vector_id - virtual interrupt vector number
 */
void env_unregister_isr(uint32_t vector_id)
{
    RL_ASSERT(vector_id < ISR_COUNT);
    if (vector_id < ISR_COUNT)
    {
        isr_table[vector_id].data = ((void *)0);
    }
}

/*!
 * env_enable_interrupt
 *
 * Enables the given interrupt
 *
 * @param vector_id   - virtual interrupt vector number
 */

void env_enable_interrupt(uint32_t vector_id)
{
    (void)platform_interrupt_enable(vector_id);
}

/*!
 * env_disable_interrupt
 *
 * Disables the given interrupt
 *
 * @param vector_id   - virtual interrupt vector number
 */

void env_disable_interrupt(uint32_t vector_id)
{
    (void)platform_interrupt_disable(vector_id);
}

/*!
 * env_map_memory
 *
 * Enables memory mapping for given memory region.
 *
 * @param pa   - physical address of memory
 * @param va   - logical address of memory
 * @param size - memory size
 * param flags - flags for cache/uncached  and access type
 */

void env_map_memory(uint32_t pa, uint32_t va, uint32_t size, uint32_t flags)
{
    platform_map_mem_region(va, pa, size, flags);
}

/*!
 * env_disable_cache
 *
 * Disables system caches.
 *
 */

void env_disable_cache(void)
{
    platform_cache_all_flush_invalidate();
    platform_cache_disable();
}

/*========================================================= */
/* Util data / functions for BM */

void env_isr(uint32_t vector)
{
    struct isr_info *info;
    RL_ASSERT(vector < ISR_COUNT);
    if (vector < ISR_COUNT)
    {
        info = &isr_table[vector];
        virtqueue_notification((struct virtqueue *)info->data);
    }
}

/*
 * env_create_queue
 *
 * Creates a message queue.
 *
 * @param queue -  pointer to created queue
 * @param length -  maximum number of elements in the queue
 * @param element_size - queue element size in bytes
 * @param queue_static_storage - pointer to queue static storage buffer
 * @param queue_static_context - pointer to queue static context
 *
 * @return - status of function execution
 */
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
int32_t env_create_queue(void **queue,
                         int32_t length,
                         int32_t element_size,
                         uint8_t *queue_static_storage,
                         rpmsg_static_queue_ctxt *queue_static_context)
#else
int32_t env_create_queue(void **queue, int32_t length, int32_t element_size)
#endif
{
    /*@queue won't be invalid, its a local variable*/
#if defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1)
    env_queue_t *que = queue_static_context;
#else
    env_queue_t *que = env_allocate_memory(sizeof(env_queue_t));
#endif
    if (!que)
        return 1;
    int res = PRT_QueueCreate(length, element_size, que);
    *queue = que;

    return res;
}

/*!
 * env_delete_queue
 *
 * Deletes the message queue.
 *
 * @param queue - queue to delete
 */

void env_delete_queue(void *queue)
{
    PRT_QueueDelete(*(env_queue_t *)queue);
#if !(defined(RL_USE_STATIC_API) && (RL_USE_STATIC_API == 1))
    env_free_memory(queue);
#endif
}

/*!
 * env_put_queue
 *
 * Put an element in a queue.
 *
 * @param queue - queue to put element in
 * @param msg - pointer to the message to be put into the queue
 * @param timeout_ms - timeout in ms
 *
 * @return - status of function execution
 */

int32_t env_put_queue(void *queue, void *msg, uintptr_t timeout_ms)
{
    /*avoid using this in ISR*/
    /*FIX:PRT_queue considers a return of 0 to be a success, the logic of rpmsglite env queue is the opposite.*/
    /*FIX:Uniproton use priority queue, while rpmsglite use it as normal queue*/
    return ((int32_t)PRT_QueueWrite(*(env_queue_t *)queue,
                                    msg,
                                    sizeof(rpmsg_queue_rx_cb_data_t),
                                    (U32)timeout_ms,
                                    OS_QUEUE_NORMAL) == 0
                ? 1
                : 0);
}

/*!
 * env_get_queue
 *
 * Get an element out of a queue.
 *
 * @param queue - queue to get element from
 * @param msg - pointer to a memory to save the message
 * @param timeout_ms - timeout in ms
 *
 * @return - status of function execution
 */

int32_t env_get_queue(void *queue, void *msg, uintptr_t timeout_ms)
{
    /*avoid using this in ISR*/
    U32 len = sizeof(rpmsg_queue_rx_cb_data_t);
    /*FIX:Uniproton considers a return of 0 to be a success, the logic of rpmsglite is the opposite.*/
    return (
        (int32_t)PRT_QueueRead(*(env_queue_t *)queue, msg, &len, (U32)timeout_ms) == 0
            ? 1
            : 0);
}

/*!
 * env_get_current_queue_size
 *
 * Get current queue size.
 *
 * @param queue - queue pointer
 *
 * @return - Number of queued items in the queue
 */

int32_t env_get_current_queue_size(void *queue)
{
    /*PRT queue has member to store used items(readcount or peak), but doesn't expose*/
    return 0;
}