/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-1-24
 * Description: I40e网卡功能适配
 */

#ifndef I40ECS_H
#define I40ECS_H

#include <linux/types.h>
#include "errno.h"
#include "i40e_type.h"

#define I40E_QTX_CTL_VF_QUEUE   0x0
#define I40E_QTX_CTL_VM_QUEUE   0x1
#define I40E_QTX_CTL_PF_QUEUE   0x2

#define IXL_ITR_NONE 3

#define I40E_TXD_QW1_TX_BUF_SZ_SHIFT   34
#define I40E_TXD_QW1_CMD_SHIFT   4

#define IXL_FILTER_USED    (U16)(1 << 0)
#define IXL_FILTER_VLAN    (U16)(1 << 1)
#define IXL_FILTER_ADD     (U16)(1 << 2)
#define IXL_FILTER_DEL     (U16)(1 << 3)
#define IXL_FILTER_MC      (U16)(1 << 4)

/* used int the vlan field of the filter when not a vlan*/
#define IXL_VLAN_ANY           (-1) 
#define IXL_AQ_LEN              256 
#define IXL_AQ_BUFSZ           4096
#define IXL_NVM_VERSION_LO_SHIFT         0
#define IXL_NVM_VERSION_LO_MASK          (0xff << IXL_NVM_VERSION_LO_SHIFT)
#define IXL_NVM_VERSION_HI_SHIFT         12
#define IXL_NVM_VERSION_HI_MASK          (0xf << IXL_NVM_VERSION_HI_SHIFT)

struct dma_map {
    U64 paddr;
    U64 vaddr;
    U64 size;
};

#define TX_BUF_SIZE 2048
#define RX_BUF_LEN 2048
#define RING_DESC_NUM 128

struct i40ecs_tx_buf {
    U8 buf[TX_BUF_SIZE];
    U32 size;
};

struct i40ecs_rx_buf {
    U8 buf[RX_BUF_LEN];
};

struct tx_ring {
    struct i40ecs_queue *que;
    U32 tail;
    U32 head;
    U16 count;
    struct dma_map *desc;
    struct dma_map *buf;
};

struct rx_ring {
    struct i40ecs_queue *que; /* back */
    U32 tail;
    U16 count;
    U32 next_to_use;
    U32 next_to_clean;
    struct dma_map *desc;
    struct dma_map *buf;
};

struct i40ecs_queue {
    struct i40ecs_vsi *vsi;
    U32                me;
    int                num_desc;
    struct tx_ring     txr;
    struct rx_ring     rxr;
};

#define MAC_ADDR_LEN 6
struct i40ecs_mac_addr {
    U8 mac[MAC_ADDR_LEN];
};
struct i40ecs_mac_filter {
    U8 mac[MAC_ADDR_LEN];
    S16 vlan;
    U16 flags;
};

#define I40E_MAC_FILTER_LEN 10
struct i40ecs_q_vector {
    struct i40ecs_vsi *vsi;
    U16 reg_idx;
};

struct i40ecs_vsi {
    struct i40ecs_pf     *pf;
    //struct device       *dev; // ***屏蔽掉
    struct i40e_hw       *hw;
    // struct i40ecs_queue    queues[1]; // *** 数组传不到rtos里面
    struct i40ecs_queue *queues;
    struct i40ecs_q_vector q_vectors[1];
    int                   num_q_vectors;
    int                   id;
    U16                   num_queues;
    U16                   seid;
    U16                   max_frame_size;
    U16                   num_vlans;

    /* MAC/VLAN Filter list */
    struct i40ecs_mac_filter mac_filter[I40E_MAC_FILTER_LEN];
    struct i40e_aqc_vsi_properties_data info;
};

struct i40e_hmc_info;

struct dma_region {
    struct dma_map tx_desc;
    struct dma_map tx_ring_buf;
    struct dma_map rx_desc;
    struct dma_map rx_ring_buf;
    struct dma_map pf_addr; /* struct of pf */
};

#define i40e_rx_desc i40e_16byte_rx_desc

#define IXL_RX_ITR     0
#define IXL_TX_ITR     1
#define IXL_QUEUE_EOL       0x7FF

#define I40E_MIN_MSIX 2
#define I40E_ITR_DYNAMIC   0x8000  /* use top bit as a flag */
#define I40E_ITR_MASK      0x1FFE  /* mask for ITR register value */
#define I40E_MIN_ITR            2  /* reg uses 2 usec resolution */
#define I40E_ITR_100K          10  /* all value below must be even */
#define I40E_ITR_50K           20
#define I40E_ITR_20K           50
#define I40E_ITR_18K           60
#define I40E_ITR_8K           122
#define I40E_MAX_ITR         8160
#define ITR_TO_REG(setting)      ((setting) & ~I40E_ITR_DYNAMIC)
#define ITR_REG_ALIGN(setting)   __ALIGN_MASK(setting, ~I40E_ITR_MASK)
#define ITR_IS_DYNAMIC(setting)  (!!((setting) & I40E_ITR_DYNAMIC))

#define I40E_ITR_RX_DEF          (I40E_ITR_20K | I40E_ITR_DYNAMIC)
#define I40E_ITR_TX_DEF          (I40E_ITR_20K | I40E_ITR_DYNAMIC)

struct msix_entry {
    u32 vector; /* kernel uses to write allocated vector */
    u16 entry;  /* Driver uses to specify entry, OS writes */
};

struct i40ecs_pf {
    /* used for both uvp and rtos */
    struct i40e_hw hw;
    struct i40ecs_rtos_pf *rpf;

    /* used for vsi msix q_vector */
    U16 num_lan_msix;
    struct msix_entry msix_entries[I40E_MIN_MSIX];

    /* used for kernel */
    void *pdev;
};

struct i40ecs_rtos_pf {
    unsigned long long hw_addr;
    struct i40ecs_vsi vsi;
    struct dma_region dma;
    struct i40e_hw hw; /* copy from kernel pf */
};

#define I40ECS_PF_MEM_ALIGN 4096
#define RESERVE_RTOS_MEM_BASE 0xf00200000ULL /* 与mmu table中的dma va地址保持一致 */
#define TX_DESC_BASE RESERVE_RTOS_MEM_BASE
#define TX_DESC_SIZE ALIGN(sizeof(struct i40e_tx_desc) * RING_DESC_NUM + sizeof(U32), I40ECS_PF_MEM_ALIGN)
#define TX_RING_BASE (TX_DESC_BASE + TX_DESC_SIZE)
#define TX_RING_SIZE ALIGN(sizeof(struct i40ecs_tx_buf) * RING_DESC_NUM, I40ECS_PF_MEM_ALIGN)
#define RX_DESC_BASE (TX_RING_BASE + TX_RING_SIZE)
#define RX_DESC_SIZE ALIGN(sizeof(union i40e_rx_desc) * RING_DESC_NUM, I40ECS_PF_MEM_ALIGN)
#define RX_RING_BASE (RX_DESC_BASE + RX_DESC_SIZE)
#define RX_RING_SIZE ALIGN(sizeof(struct i40ecs_rx_buf) * RING_DESC_NUM, I40ECS_PF_MEM_ALIGN)
#define RPF_BASE (RX_RING_BASE + RX_RING_SIZE)
#define RPF_SIZE sizeof(struct i40ecs_rtos_pf)
#define QUEUE_BASE (RPF_BASE + RPF_SIZE)
#define QUEUE_SIZE sizeof(struct i40ecs_queue)
#define RPF_TOTAL_SIZE (QUEUE_BASE + QUEUE_SIZE - RESERVE_RTOS_MEM_BASE)

static inline void plat_writel(U32 val, U32 *addr)
{
    *addr = val;
}

static inline U32 plat_readl(U32 *addr)
{
    return *addr;
}

#define mac_write(a, reg, value)   plat_writel((value), (U32*)(size_t)((a)->hw_addr + (reg)))
#define mac_read(a, reg)           plat_readl((U32*)(size_t)((a)->hw_addr + (reg)))
#define i40ecs_flush(a)            plat_readl((U32*)(size_t)((a)->hw_addr + I40E_GLGEN_STAT))

void i40e_init(void);
int i40e_packet_send(u8 pf_id, const unsigned char *packet, int length);
int i40e_packet_recv(u8 pf_id, unsigned char *packet, int length);
void i40ecs_test_start(void);
void i40ecs_test_stop(void);

#endif