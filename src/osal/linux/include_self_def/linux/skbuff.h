/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _LINUX_SKBUFF_H
#define _LINUX_SKBUFF_H

#include <linux/netdevice.h>
#include <string.h>
#include "prt_err.h"
#include "prt_mem.h"
#include "prt_module.h"
#include "stddef.h"
#include "types.h"
#include <linux/netdevice.h>
#include <linux/minmax.h>

#ifdef NET_SKBUFF_DATA_USES_OFFSET
typedef unsigned int sk_buff_data_t;
#else
typedef unsigned char *sk_buff_data_t;
#endif

#define CONFIG_X86_L1_CACHE_SHIFT 6
#define L1_CACHE_SHIFT (CONFIG_X86_L1_CACHE_SHIFT)
#define L1_CACHE_BYTES (1 << L1_CACHE_SHIFT)

/*
 * The networking layer reserves some headroom in skb data (via
 * dev_alloc_skb). This is used to avoid having to reallocate skb data when
 * the header has to grow. In the default case, if the header has to grow
 * 32 bytes or less we avoid the reallocation.
 *
 * Unfortunately this headroom changes the DMA alignment of the resulting
 * network packet. As for NET_IP_ALIGN, this unaligned DMA is expensive
 * on some architectures. An architecture can override this value,
 * perhaps setting it to a cacheline in size (since that will maintain
 * cacheline alignment of the DMA). It must be a power of 2.
 *
 * Various parts of the networking layer expect at least 32 bytes of
 * headroom, you should not reduce this.
 *
 * Using max(32, L1_CACHE_BYTES) makes sense (especially with RPS)
 * to reduce average number of cache lines per packet.
 * get_rps_cpu() for example only access one 64 bytes aligned block :
 * NET_IP_ALIGN(2) + ethernet_header(14) + IP_header(20/40) + ports(8)
 */
#ifndef NET_SKB_PAD
#define NET_SKB_PAD max(32, L1_CACHE_BYTES)
#endif

#ifndef SMP_CACHE_BYTES
#define SMP_CACHE_BYTES L1_CACHE_BYTES
#endif

#define SKB_DATA_ALIGN(X) ALIGN(X, SMP_CACHE_BYTES)

/* return minimum truesize of one skb containing X bytes of data */
#define SKB_TRUESIZE(X) ((X) + SKB_DATA_ALIGN(sizeof(struct sk_buff)) + SKB_DATA_ALIGN(sizeof(struct skb_shared_info)))

struct net_device;

struct sk_buff {
    union {
        struct net_device *dev;
        unsigned long dev_scratch;
    };
    unsigned int len, data_len;
    sk_buff_data_t tail;
    sk_buff_data_t end;
    unsigned char *head, *data;
    unsigned int truesize;
};

/* This data is invariant across clones and lives at
 * the end of the header data, ie. at skb->end.
 */
struct skb_shared_info { };

#ifdef NET_SKBUFF_DATA_USES_OFFSET
static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
    skb->tail = skb->data - skb->head;
}

static inline void skb_set_end_offset(struct sk_buff *skb, unsigned int offset)
{
    skb->end = offset;
}
#else
static inline void skb_reset_tail_pointer(struct sk_buff *skb)
{
    skb->tail = skb->data;
}

static inline void skb_set_end_offset(struct sk_buff *skb, unsigned int offset)
{
    skb->end = skb->head + offset;
}
#endif

/**
 *    skb_reserve - adjust headroom
 *    @skb: buffer to alter
 *    @len: bytes to move
 *
 *    Increase the headroom of an empty &sk_buff by reducing the tail
 *    room. This is only allowed for an empty buffer.
 */
static inline void skb_reserve(struct sk_buff *skb, int len)
{
    skb->data += len;
    skb->tail += len;
}

struct sk_buff *__netdev_alloc_skb(struct net_device *dev, unsigned int len, gfp_t gfp_mask);
void consume_skb(struct sk_buff *skb);
void *skb_push(struct sk_buff *skb, unsigned int len);

#define dev_kfree_skb(a)    consume_skb(a)

/**
 *    netdev_alloc_skb - allocate an skbuff for rx on a specific device
 *    @dev: network device to receive on
 *    @length: length to allocate
 *
 *    Allocate a new &sk_buff and assign it a usage count of one. The
 *    buffer has unspecified headroom built in. Users should allocate
 *    the headroom they think they need without accounting for the
 *    built in space. The built in space is used for optimisations.
 *
 *    %NULL is returned if there is no free memory. Although this function
 *    allocates memory it can be called from an interrupt.
 */
static inline struct sk_buff *netdev_alloc_skb(struct net_device *dev,
                           unsigned int length)
{
    return __netdev_alloc_skb(dev, length, GFP_ATOMIC);
}

/* legacy helper around netdev_alloc_skb() */
static inline struct sk_buff *dev_alloc_skb(unsigned int length)
{
    return netdev_alloc_skb(NULL, length);
}

#endif