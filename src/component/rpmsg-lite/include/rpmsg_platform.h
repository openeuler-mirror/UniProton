/*
 * Copyright 2022-2023 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef RPMSG_PLATFORM_H_
#define RPMSG_PLATFORM_H_

#include <stdint.h>

struct rpmsg_platform_shmem_config
{
    uint32_t buffer_payload_size; /* custom buffer payload size setting that overwrites RL_BUFFER_PAYLOAD_SIZE global
                                     config, must be equal to (240, 496, 1008, ...) [2^n - 16] */
    uint16_t buffer_count; /* custom buffer count setting that overwrites RL_BUFFER_COUNT global config, must be power
                              of two (2, 4, ...) */
    uint32_t vring_size;   /* custom vring size */
    uint32_t vring_align;  /* custom vring alignment */
};
typedef struct rpmsg_platform_shmem_config rpmsg_platform_shmem_config_t;

#define UNI2UNI_RPMSGLITE_TEST_LINKID 0
/*
 * Linux requires the ALIGN to 0x1000(4KB) instead of 0x80
 */
#ifndef VRING_ALIGN
#define VRING_ALIGN (0x1000U)
#endif

/* contains pool of descriptors and two circular buffers */
#ifndef VRING_SIZE
#define VRING_SIZE (0x8000UL)
#endif

/* define shared memory space for VRINGS per one channel */
#define RL_VRING_OVERHEAD (2UL * VRING_SIZE)

#define RL_GET_VQ_ID(link_id, queue_id) (((queue_id)&0x1U) | (((link_id) << 1U) & 0xFFFFFFFEU))
#define RL_GET_LINK_ID(id)              (((id)&0xFFFFFFFEU) >> 1U)
#define RL_GET_Q_ID(id)                 ((id)&0x1U)

#define RL_PLATFORM_USER_LINK_ID    (0U)
#define RL_PLATFORM_HIGHEST_LINK_ID (15U)

/* platform interrupt related functions */
int32_t platform_init_interrupt(uint32_t vector_id, void *isr_data);
int32_t platform_deinit_interrupt(uint32_t vector_id);
int32_t platform_interrupt_enable(uint32_t vector_id);
int32_t platform_interrupt_disable(uint32_t vector_id);
void platform_notify(uint32_t vector_id);

/* platform low-level time-delay (busy loop) */
void platform_time_delay(uint32_t num_msec);

/* platform memory functions */
void platform_map_mem_region(uint32_t vrt_addr, uint32_t phy_addr, uint32_t size, uint32_t flags);
void platform_cache_all_flush_invalidate(void);
void platform_cache_disable(void);
uintptr_t platform_vatopa(void *addr);
void *platform_patova(uintptr_t addr);

/* platform init/deinit */
int32_t platform_init(void);
int32_t platform_deinit(void);

/*custom shmem config*/
int32_t platform_get_custom_shmem_config(uint32_t link_id, rpmsg_platform_shmem_config_t *cfg);
#endif /* RPMSG_PLATFORM_H_ */
