/*
 * Copyright (c) 2023-2023 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2023-11-30
 * Description: ITS驱动
 */

#ifndef __PRT_GIC_ITS_H__
#define __PRT_GIC_ITS_H__

#include "prt_typedef.h"
#include "its_list.h"
#include "prt_buildef.h"
#include "securec.h"

extern U32 PRT_Printf(const char *format, ...);

/* ITS registers, offset form ITS_base*/
#define GITS_CTLR               0x0000U
#define GITS_TYPER              0x0008U
#define GITS_CBASER             0x0080U
#define GITS_CWRITE             0x0088U
#define GITS_CREADER            0x0090U
#define GITS_BASER              0x0100U
#define GITS_PIDR2              0xFFE8U

#define GITS_CTLR_ENABLE_MASK   1U
#define GITS_CTLR_ENABLE        1U
#define GITS_CTLR_QUIET         (1U << 31U)

#define GITS_TYPER_ITE_VIRTUAL_SHIFT    1U
#define GITS_TYPER_ITE_VIRTUAL(r)       (((r) >> GITS_TYPER_ITE_VIRTUAL_SHIFT) & 0x1U)
#define GITS_TYPER_ITE_SIZE_SHIFT       4U
#define GITS_TYPER_ITE_SIZE_MASK        0xF0
#define GITS_TYPER_ITE_SIZE(r)          ((((r) >> GITS_TYPER_ITE_SIZE_SHIFT) & 0x1FU) + 1U)
#define GITS_TYPER_ITE_ID_BIT_SHIFT     8U
#define GITS_TYPER_DEVID_BITS_SHIFT     13U
#define GITS_TYPER_DEVID_BITS(r)        ((((r) >> GITS_TYPER_DEVID_BITS_SHIFT) & 0x1FU) + 1U)
#define GITS_TYPER_PTA_BITS_SHIFT       19U
#define GITS_TYPER_PTA_BITS(r)          (((r) >> GITS_TYPER_PTA_BITS_SHIFT) & 0x1U)
#define GITS_TYPER_PHYSICAL             (1U << 0U)
#define GITS_TYPER_HCC_SHIFT            24U
#define GITS_TYPER_HCC_MASK             (0xFFU << 24U)

/*CBASER 相关*/
#define GITS_CBASER_VALID_SHIFT         63U
#define GITS_CBASER_VALID_MASK          (1ULL << GITS_CBASER_VALID_SHIFT)
#define GITS_CBASER_VALID               (GITS_CBASER_VALID_MASK)
#define GITS_CBASER_IN_CACHE_SHIFT      59U
#define GITS_CBASER_IN_CACHE(r)         (((r) >> GITS_CBASER_IN_CACHE_SHIFT) & 0x7U)
#define GITS_CBASER_OUT_CACHE_SHIFT     53U
#define GITS_CBASER_OUT_CACHE(r)        (((r) >> GITS_CBASER_OUT_CACHE_SHIFT) & 0x7U)
#define GITS_CBASER_PHYADDR_MASK        0xFFFFFFFFFF000ULL
#define GITS_CBASER_SHAREABILITY_SHIFT  10U
#define GITS_CBASER_SIZE_MASK           0xFFULL

/*CWRITE 相关*/
#define GITS_CWRITE_OFFSET_MASK         (0x7FFFU << 5U)

/*CREADER 相关*/
#define GITS_CREADER_OFFSET_SHIFT       5U

/*BASER 相关*/
#define GITS_BASER_VALID_MASK           (1ULL << 63U)
#define GITS_BASER_VALID                (GITS_BASER_VALID_MASK)
#define GITS_BASER_IN_CACHE_SHIFT       59U
#define GITS_BASER_IN_CACHE_MASK        (0x7ULL << GITS_BASER_IN_CACHE_SHIFT)
#define GITS_BASER_IN_CACHE(r)          (((r) >> GITS_BASER_IN_CACHE_SHIFT) & 0x7U)
#define GITS_BASER_TYPE_SHIFT           56U
#define GITS_BASER_TYPE(r)              (((r) >> GITS_BASER_TYPE_SHIFT) & 0x7U)
#define GITS_BASER_OUT_CACHE_SHIFT      53U
#define GITS_BASER_OUT_CACHE_MASK       (0x7ULL << GITS_BASER_OUT_CACHE_SHIFT)
#define GITS_BASER_ENTRY_SIZE_SHIFT     48U
#define GITS_BASER_ENTRY_SIZE(r)        ((((r) >> GITS_BASER_ENTRY_SIZE_SHIFT) & 0x1FU) + 1U)
#define GITS_BASER_PHY_ADDR_MASK        0xFFFFFFFFF000ULL
#define GITS_BASER_SHAREABILITY_SHIFT   10U
#define GITS_BASER_SHAREABILITY(r)      (((r) >> GITS_BASER_SHAREABILITY_SHIFT) & 0x3U)
#define GITS_BASER_PAGE_SIZE_SHIFT      8U
#define GITS_BASER_PAGE_SIZE(r)         (((r) >> GITS_BASER_PAGE_SIZE_SHIFT) & 0x3U)
#define GITS_BASER_PAGE_NR_SHIFT        0U
#define GITS_BASER_PAGE_NR_MASK         0xFFULL
#define GITS_BASER_PAGE_SZ_4K           0U
#define GITS_BASER_PAGE_SZ_16K          1U
#define GITS_BASER_PAGE_SZ_64K          2U
#define GITS_BASER_PAGE_SZ_RESERVE      3U

#define GITS_BASER_TYPE_NONE            0ULL
#define GITS_BASER_TYPE_DEVICE          1ULL
#define GITS_BASER_TYPE_VPES            2ULL
#define GITS_BASER_TYPE_RESERVED3       3ULL
#define GITS_BASER_TYPE_IRQ_COLLECT     4ULL
#define GITS_BASER_TYPE_RESERVED5       5ULL
#define GITS_BASER_TYPE_RESERVED6       6ULL
#define GITS_BASER_TYPE_RESERVED7       7ULL

/*ITS commands*/
#define GITS_CMD_ENTRY_SIZE             32U

#define GITS_CMD_BYTE_ORDER_FIRST       0U
#define GITS_CMD_BYTE_ORDER_SECOND      1U
#define GITS_CMD_BYTE_ORDER_THIRD       2U
#define GITS_CMD_BYTE_ORDER_FOURTH      3U

#define GITS_CMD_FIELD_SHIFT_DEVICE_ID  32U
#define GITS_CMD_FIELD_SHIFT_VALID      63U
#define GITS_CMD_FIELD_SHIFT_PINTID     32U
#define GITS_CMD_FIELD_SHIFT_ITT_ADDR   32U
#define GITS_CMD_FIELD_SHIFT_VPE_ID     32U
#define GITS_CMD_FIELD_SHIFT_DB_PINT_ID 32U
#define GITS_CMD_FIELD_SHIFT_SEQ_NUM    32U

#define GITS_CMD_CMD_CODE_CLEAR         0x4ULL
#define GITS_CMD_CMD_CODE_DISCARD       0xFULL
#define GITS_CMD_CMD_CODE_INT           0x3ULL
#define GITS_CMD_CMD_CODE_INV           0xCULL
#define GITS_CMD_CMD_CODE_MAPI          0xBULL
#define GITS_CMD_CMD_CODE_MOVALL        0xEULL
#define GITS_CMD_CMD_CODE_MOVI          0x1ULL
#define GITS_CMD_CMD_CODE_MAPD          0x8uLL
#define GITS_CMD_CMD_CODE_MAPC          0x9ULL
#define GITS_CMD_CMD_CODE_MAPTI         0xAULL
#define GITS_CMD_CMD_CODE_INVALL        0xDULL
#define GITS_CMD_CMD_CODE_SYNC          0x5ULL

#define GITS_CMD_FIELD_MASK_CMD_CODE    0xFFULL
#define GITS_CMD_FIELD_MASK_SIZE        0x1FULL
#define GITS_CMD_FIELD_MASK_ITT_ADDR    0xFFFFFFFFFFF00ULL
#define GITS_CMD_FIELD_MASK_RDBASE      0x7FFFFFFFF0000ULL
#define GITS_CMD_FIELD_MASK_ICID        0xFFFFULL
#define GITS_CMD_FIELD_MASK_EVENTID     0xFFFFFFFFULL
#define GITS_CMD_FIELD_MASK_VALID       (0x1ULL << 63U)
#define GITS_CMD_FIELD_MASK_DEVICE_ID   0xFFFFFFFF00000000ULL
#define GITS_CMD_FIELD_MASK_PINTID      0xFFFFFFFF00000000ULL
#define GITS_CMD_FIELD_MASK_VINTID      0xFFFFFFFFULL
#define GITS_CMD_FIELD_MASK_VPT_ADDR    0x7FFFFFFFF0000ULL
#define GITS_CMD_FIELD_MASK_VPT_SIZE    0x1FULL
#define GITS_CMD_FIELD_MASK_D_FLAG      0x1ULL
#define GITS_CMD_FIELD_MASK_ITS_LIST    0xFFFFULL

/*CACHE属性*/
#define GITS_CACHE_RaWaWb               7ULL


#define GITS_BASER_InnerShareable       1ULL

#define GITS_SHARE_NonShareable         0ULL
#define GITS_SHARE_InnerShareable       1ULL
#define GITS_SHARE_OuterShareable       2ULL

/*GICR相关*/
#define GITS_GICR_CTLR                  0x0000U
#define GITS_GICR_CTLR_ENABLE_LPI       0x1

#define GICV3_GICR_PENDBASER_SHAREABILITY_SHIFT     10U
#define GICV3_GICR_PENDBASER_CACHEABILITY_SHIFT     7U
#define GICV3_GICR_PROPBASER_SHAREABILITY_SHIFT     10U
#define GICV3_GICR_PROPBASER_CACHEABILITY_SHIFT     7U

#define GICV3_GICR_PENDBASER            0x0078U
#define GICV3_GICR_PROPBASER            0x0070U

#define GICV3_GICR_PROPBASER_IDBITS     0x1FU

/*其他*/
#define GITS_CMDQ_MEM_ALIGN             0x10000U
#define GITS_CMD_QUEUE_SIZE             0x1000U

#define GITS_PEND_TABLE_ALIGN           0x10000U
#define GITS_PEND_TABLE_SIZE            0x2000U

#define GITS_PROP_TABLE_SIZE            (0x10000U - 0x2000U)
#define GITS_PROP_TABLE_ALIGN           0x1000U

#define GITS_LPI_PROP_GROUP1            (1U << 1U)
#define GITS_LPI_PROP_ENABLE_MASK       0x1U
#define GITS_LPI_PROP_ENABLE            0x1U

#define GITS_LPI_DEFAULT_PRIO           0xE0U
#define GITS_LPI_INVALID_HWIRQ          0x7FFFFFFFU

#define GITS_ITT_MEM_ALIGN              256U

#define GITS_COLLECTION_ID_WIDTH        8U

#define GITS_BASER_REG_NR               8U

#define GITS_LPI_HWIRQ_MIN              8192U

/*一些根据配置定义的宏，暂时在这里定义*/
#define GITS_MAX_HWIRQ                  20000U

struct its_attribute_s {
    U32 device_id_bit;      // 用几个bit表示device ID
    U32 itt_entry_size;     // ITT 条目的大小
    U32 pta;
    U8 virtual;
};

struct its_resource_s {
    void *dt_addr;
    void *ct_addr;
    void *vpet_addr;
    void *cmdq_base;
    void *prop_table_addr;
    void *pending_table_addr[OS_MAX_CORE_NUM];
};

struct its_device_s {
    void *base_addr;
    U32 cmdq_wr_index;
    U64 rd_phy[OS_MAX_CORE_NUM];
    void *rd_virt[OS_MAX_CORE_NUM];
    unsigned int hwirq_max;     // 中断最大值
    struct its_attribute_s its_attr;
    struct its_resource_s its_res;
    struct list_node dev_info_head;
};

struct its_device_info_s {
    U32 device_id;
    U32 hwirq_base;
    U32 event_nr;
    struct list_node list;
};

struct its_cmd_int_arg_s {
    U64 device_id;
    U64 event_id;
};

struct its_cmd_clear_arg_s {
    U64 device_id;
    U64 event_id;
};

struct its_cmd_inv_arg_s {
    U64 device_id;
    U64 event_id;
};

struct its_cmd_mapd_arg_s {
    U64 device_id;
    U64 size;
    U64 itt_addr;
    U64 valid;
};

struct its_cmd_mapc_arg_s {
    U64 icid;
    U64 target_addr;
    U64 valid;
};

struct its_cmd_mapti_arg_s {
    U64 device_id;
    U64 event_id;
    U64 icid;
    U64 int_id;
};

struct its_cmd_invall_arg_s {
    U64 icid;
};

struct its_cmd_sync_arg_s {
    U64  target_addr;
};

union its_cmd_arg_s {
    struct its_cmd_int_arg_s cmd_int;
    struct its_cmd_clear_arg_s cmd_clear;
    struct its_cmd_inv_arg_s cmd_inv;
    struct its_cmd_mapd_arg_s cmd_mapd;
    struct its_cmd_mapc_arg_s cmd_mapc;
    struct its_cmd_mapti_arg_s cmd_mapti;
    struct its_cmd_invall_arg_s cmd_invall;
    struct its_cmd_sync_arg_s cmd_sync;
};

struct redist_addr_info_s {
    U64 phy_addr;
    void *virt_addr;
};

typedef unsigned long uptr_t;
typedef void (*its_fill_cmd_fn) (U64 *cmd, union its_cmd_arg_s *_cmd_arg);

#define dsb(opt)                __asm__ volatile("dsb " #opt : : : "memory")

static inline U64 __read64(volatile void *addr)
{
    U64 val;
     __asm__ volatile("ldar %x0, [%1]" : "=r" (val) : "r" (addr));
    return val;
}

static inline void __write64(volatile void *addr, U64 val)
{
    U64 __val = val;
     __asm__ volatile("str %x0, [%1]" : : "rZ" (__val), "r" (addr));
}

static inline U64 read64(void *address)
{
    U64 value = __read64(address);
    dsb(ld);
    return value;
}

static inline void write64(void *address, U64 val)
{
    dsb(st);
    __write64(address, val);
}

static inline void *io_reg(void *base_addr, unsigned long offset)
{
    U8 *addr = (U8 *)(uptr_t)(base_addr);
    return &addr[offset];
}

static inline U32 read32(void *address)
{
    return *(volatile U32 *)(uintptr_t)address;
}

static inline void write32(void *address, U32 val)
{
    *(volatile U32 *)(uintptr_t)(address) =  (U32)val;
}

static inline U32 io_reg_read32(void *base_addr, unsigned long offset)
{
    return read32(io_reg(base_addr, offset));
}

static inline void io_reg_write32(void *base_addr, unsigned long offset, U32 val)
{
    write32(io_reg(base_addr, offset), val);
}

static inline U64 io_reg_read64(void *base_addr, unsigned long offset)
{
    return read64(io_reg(base_addr, offset));
}

static inline void io_reg_write64(void *base_addr, unsigned long offset, U64 val)
{
    write64(io_reg(base_addr, offset), val);
}

static inline void cpu_relax(void)
{
     __asm__ volatile("" ::: "memory");
}

static inline void __bzero(void *dst, size_t size)
{
    (void)memset_s(dst, size, 0, size);
}

#define mem_zero_b(mem, len)            __bzero((void *)(mem), (len))
#define mem_zero_s(data)                __bzero((void *)(&(data)), sizeof(data))

static inline unsigned int __fls(U32 v)
{
    return (v == 0U) ? 0U : (unsigned int)(sizeof(v) * 8U - (unsigned int)__builtin_clz((unsigned int)(v)));
}

#define ptr_to_U64(ptr)                 ((U64)(uptr_t)(ptr))
#define U64_to_ptr(val, type)           ((type *)((uptr_t)(val)))

#define ALIGN_UP(val, align)            (((val) + ((align) - 1UL)) & ~((align) - 1UL))

void its_set_disable(void *base_addr);
void its_set_enable(void *base_addr);
int its_set_irq_pending(struct its_device_s *its_device, U32 hwirq);
int its_clear_irq_pending(struct its_device_s *its_device, U32 hwirq);

int its_alloc_its_tables(struct its_device_s *its_device);
int its_init_cmd_queue(struct its_device_s *its_device);
int its_map_icid_to_redist(struct its_device_s *its_device, struct redist_addr_info_s *gicr_addr, U32 icid);
int its_set_irq_enable(struct its_device_s *its_device, U32 hwirq);
int its_set_irq_disable(struct its_device_s *its_device, U32 hwirq);
void its_device_init(struct its_device_s *its_device);
void its_device_info_init(struct its_device_s *its_device, struct its_device_info_s *device_list, U32 device_num);
void its_setup_device_table(struct its_device_s *its_device, U32 collect_id);
#endif
