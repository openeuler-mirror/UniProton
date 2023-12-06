#include <stdarg.h>
#include "prt_gic_its.h"
#include "its_list.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "stdlib.h"
#include "cpu_config.h"
#include "test.h"

static void *boot_alloc_align(size_t size, size_t align)
{
    return PRT_MemAllocAlign(0, OS_MEM_DEFAULT_FSC_PT, size, align);
}

static inline void its_cmd_fill_cmd_code(U64 *cmd, U64 cmd_code)
{
    cmd[GITS_CMD_BYTE_ORDER_FIRST] = cmd[GITS_CMD_BYTE_ORDER_FIRST] |
        (cmd_code & GITS_CMD_FIELD_MASK_CMD_CODE);
}

static inline void its_cmd_fill_device_id(U64 *cmd, U64 device_id)
{
    cmd[GITS_CMD_BYTE_ORDER_FIRST] = cmd[GITS_CMD_BYTE_ORDER_FIRST] |
        (device_id << GITS_CMD_FIELD_SHIFT_DEVICE_ID);
}

static inline void its_cmd_fill_size(U64 *cmd, U64 size)
{
    cmd[GITS_CMD_BYTE_ORDER_SECOND] = cmd[GITS_CMD_BYTE_ORDER_SECOND] |
        (size & GITS_CMD_FIELD_MASK_SIZE);
}

static inline void its_cmd_fill_valid(U64 *cmd, U64 valid)
{
    cmd[GITS_CMD_BYTE_ORDER_THIRD] = cmd[GITS_CMD_BYTE_ORDER_THIRD] |
        (valid << GITS_CMD_FIELD_SHIFT_VALID);
}

static inline void its_cmd_fill_itt_addr(U64 *cmd, U64 itt_addr)
{
    cmd[GITS_CMD_BYTE_ORDER_THIRD] = cmd[GITS_CMD_BYTE_ORDER_THIRD] |
        (itt_addr & GITS_CMD_FIELD_MASK_ITT_ADDR);
}

static inline void its_cmd_fill_rdbase(U64 *cmd, U64 rdbase)
{
    cmd[GITS_CMD_BYTE_ORDER_THIRD] = cmd[GITS_CMD_BYTE_ORDER_THIRD] |
        (rdbase & GITS_CMD_FIELD_MASK_RDBASE);
}

static inline void its_cmd_fill_icid(U64 *cmd, U64 icid)
{
    cmd[GITS_CMD_BYTE_ORDER_THIRD] = cmd[GITS_CMD_BYTE_ORDER_THIRD] |
        (icid & GITS_CMD_FIELD_MASK_ICID);
}

static inline void its_cmd_fill_event_id(U64 *cmd, U64 event_id)
{
    cmd[GITS_CMD_BYTE_ORDER_SECOND] = cmd[GITS_CMD_BYTE_ORDER_SECOND] |
        (event_id & GITS_CMD_FIELD_MASK_EVENTID);
}

static inline void its_cmd_fill_pintid(U64 *cmd, U64 pintid)
{
    cmd[GITS_CMD_BYTE_ORDER_SECOND] = cmd[GITS_CMD_BYTE_ORDER_SECOND] |
        (pintid << GITS_CMD_FIELD_SHIFT_PINTID);
}

static inline void its_cmd_fill_seq_num(U64 *cmd, U64 seq_num)
{
    cmd[GITS_CMD_BYTE_ORDER_FIRST] = cmd[GITS_CMD_BYTE_ORDER_FIRST] |
        (seq_num << GITS_CMD_FIELD_SHIFT_SEQ_NUM);
}

static void its_cmd_mapd(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_mapd_arg_s *cmd_arg = &_cmd_arg->cmd_mapd;
    if ((U64)cmd_arg->itt_addr & 0xFFU) {
        PRT_Printf("[ERROR] ITS: bad itt address.\n");
        return;
    }

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_MAPD);
    its_cmd_fill_device_id(cmd, cmd_arg->device_id);
    its_cmd_fill_size(cmd, cmd_arg->size);
    its_cmd_fill_valid(cmd, cmd_arg->valid);
    its_cmd_fill_itt_addr(cmd, cmd_arg->itt_addr);
}

static void its_cmd_mapc(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_mapc_arg_s *cmd_arg = &_cmd_arg->cmd_mapc;
    if ((U64)cmd_arg->target_addr & 0xFFFFU) {
        PRT_Printf("[ERROR] ITS: bad target address.\n");
        return;
    }

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_MAPC);
    its_cmd_fill_valid(cmd, cmd_arg->valid);
    its_cmd_fill_rdbase(cmd, cmd_arg->target_addr);
    its_cmd_fill_icid(cmd, cmd_arg->icid);
}

static void its_cmd_mapti(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_mapti_arg_s *cmd_arg = &_cmd_arg->cmd_mapti;

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_MAPTI);
    its_cmd_fill_device_id(cmd, cmd_arg->device_id);
    its_cmd_fill_event_id(cmd, cmd_arg->event_id);
    its_cmd_fill_pintid(cmd, cmd_arg->int_id);
    its_cmd_fill_icid(cmd, cmd_arg->icid);
}

static void its_cmd_int(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_int_arg_s *cmd_arg = &_cmd_arg->cmd_int;

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_INT);
    its_cmd_fill_device_id(cmd, cmd_arg->device_id);
    its_cmd_fill_event_id(cmd, cmd_arg->event_id);
}

static void its_cmd_clear(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_clear_arg_s *cmd_arg = &_cmd_arg->cmd_clear;

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_CLEAR);
    its_cmd_fill_device_id(cmd, cmd_arg->device_id);
    its_cmd_fill_event_id(cmd, cmd_arg->event_id);
}

static void its_cmd_inv(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_inv_arg_s *cmd_arg = &_cmd_arg->cmd_inv;

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_INV);
    its_cmd_fill_device_id(cmd, cmd_arg->device_id);
    its_cmd_fill_event_id(cmd, cmd_arg->event_id);
}

static void its_cmd_invall(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_invall_arg_s *cmd_arg = &_cmd_arg->cmd_invall;

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_INVALL);
    its_cmd_fill_icid(cmd, cmd_arg->icid);
}

static void its_cmd_sync(U64 *cmd, union its_cmd_arg_s *_cmd_arg)
{
    if (_cmd_arg == NULL) {
        PRT_Printf("[ERROR] input argument is null.\n");
    }

    struct its_cmd_sync_arg_s *cmd_arg = &_cmd_arg->cmd_sync;

    its_cmd_fill_cmd_code(cmd, GITS_CMD_CMD_CODE_SYNC);
    its_cmd_fill_rdbase(cmd, cmd_arg->target_addr);
}

static void wait_cmd_complete(struct its_device_s *its_device)
{
    U64 cq_rd_index;
    do {
        cq_rd_index = io_reg_read64(its_device->base_addr, GITS_CREADER);
        cpu_relax();
    } while (cq_rd_index != its_device->cmdq_wr_index);
}

static inline void do_its_send_cmd(struct its_device_s *its_device)
{
    io_reg_write64(its_device->base_addr, GITS_CWRITE, its_device->cmdq_wr_index);
}

static U64 *its_get_cmdq_wr_addr(struct its_device_s *its_device)
{
    return (U64 *)((char *)(its_device->its_res.cmdq_base) + its_device->cmdq_wr_index);
}

static void its_update_cmdq_wr_index(struct its_device_s *its_device)
{
    its_device->cmdq_wr_index = (its_device->cmdq_wr_index + GITS_CMD_ENTRY_SIZE) % GITS_CMD_QUEUE_SIZE;
}

static void its_send_cmd(struct its_device_s *its_device,
    its_fill_cmd_fn fill_cmd_fn, union its_cmd_arg_s *cmd_arg)
{
    U64 *cmd = its_get_cmdq_wr_addr(its_device);
    mem_zero_b(cmd, GITS_CMD_ENTRY_SIZE);
    fill_cmd_fn(cmd, cmd_arg);
    its_update_cmdq_wr_index(its_device);

    if (its_device->rd_phy[0] != 0UL) {
        cmd = its_get_cmdq_wr_addr(its_device);
        mem_zero_b(cmd, GITS_CMD_ENTRY_SIZE);
        union its_cmd_arg_s sync_arg;
        sync_arg.cmd_sync.target_addr = its_device->rd_phy[0];
        its_cmd_sync(cmd, &sync_arg);
        its_update_cmdq_wr_index(its_device);
    }

    dsb(sy);

    do_its_send_cmd(its_device);
    wait_cmd_complete(its_device);
}

static void its_map_device_id_to_itt(struct its_device_s *its_device, U32 device_id, U32 event_id_range)
{
    struct its_cmd_mapd_arg_s cmd_arg;
    mem_zero_s(cmd_arg);
    U64 size = __fls(event_id_range);
    size = (size ==  0ULL) ? 1ULL : size;

    U64 mem_size = (1U << size) * its_device->its_attr.itt_entry_size;
    U64 itt_addr = ptr_to_U64(boot_alloc_align(mem_size, MEM_ADDR_ALIGN_256));

    if (itt_addr == 0ULL) {
        PRT_Printf("[ERROR] get itt address failed.\n");
        return;
    }

    cmd_arg.device_id = (U64)device_id;
    cmd_arg.itt_addr = itt_addr;
    cmd_arg.valid = 1U;
    cmd_arg.size = size - 1U;

    its_send_cmd(its_device, its_cmd_mapd, (union its_cmd_arg_s *)&cmd_arg);
}

int its_map_icid_to_redist(struct its_device_s *its_device, struct redist_addr_info_s *gicr_addr, U32 collect_id)
{
    union its_cmd_arg_s cmd_arg;
    mem_zero_s(cmd_arg);

    if (its_device->its_attr.pta != 1ULL) {
        PRT_Printf("ITS: not support PE number as target address");
        return OS_ERROR;
    }
    
    U64 target_addr = gicr_addr->phy_addr;

    cmd_arg.cmd_mapc.target_addr = target_addr;
    cmd_arg.cmd_mapc.icid = collect_id;
    cmd_arg.cmd_mapc.valid = 1ULL;
    
    its_send_cmd(its_device, its_cmd_mapc, &cmd_arg);

    cmd_arg.cmd_invall.icid = collect_id;
    its_send_cmd(its_device, its_cmd_invall, &cmd_arg);

    U32 core_id = PRT_GetCoreID();
    if (core_id > OS_MAX_CORE_NUM) {
        PRT_Printf("[ERROR] cpu core id beyonds the max number.\n");
        return OS_ERROR;
    }

    its_device->rd_phy[core_id] = gicr_addr->phy_addr;

    return OS_OK;
}

static int its_config_lpi_route(struct its_device_s *its_device, U32 device_id, U32 event_id, U32 hwirq, U32 collect_id)
{
    union its_cmd_arg_s cmd_arg;
    mem_zero_s(cmd_arg);

    cmd_arg.cmd_mapti.device_id = device_id;
    cmd_arg.cmd_mapti.event_id = event_id;
    cmd_arg.cmd_mapti.int_id = hwirq;
    cmd_arg.cmd_mapti.icid = collect_id;

    its_send_cmd(its_device, its_cmd_mapti, &cmd_arg);

    mem_zero_s(cmd_arg);
    cmd_arg.cmd_inv.device_id = device_id;
    cmd_arg.cmd_inv.event_id = event_id;
    its_send_cmd(its_device, its_cmd_inv, &cmd_arg);

    return OS_OK;
}

static U64 its_get_table_size(U32 id_width, U32 te_size, U32 align)
{
    U64 table_size = (1U << id_width) * te_size;
    table_size = ALIGN_UP(table_size, align);

    return table_size;
}

static U32 its_alloc_table_mem(U32 id_width, U32 te_size, U32 align, void **ptable)
{
    U64 table_size = its_get_table_size(id_width, te_size, align);
    void *table_buffer = boot_alloc_align(table_size, MEM_ADDR_ALIGN_4K);
    if (table_buffer ==  NULL) {
        return 0U;
    }

    mem_zero_b(table_buffer, table_size);

    *ptable = table_buffer;
    
    return table_size;
}

static void *its_alloc_cmd_queue(void)
{
    void *cmd_queue = boot_alloc_align(GITS_CMD_QUEUE_SIZE, MEM_ADDR_ALIGN_4K);
    if (cmd_queue == NULL) {
        return NULL;
    }
    mem_zero_b(cmd_queue, GITS_CMD_QUEUE_SIZE);

    return cmd_queue;
}

static void its_set_enable_disable(void *base_addr, int enable)
{
    U32 its_ctrl = io_reg_read32(base_addr, GITS_CTLR);
    its_ctrl = (enable == false) ? (its_ctrl & ~GITS_CTLR_ENABLE_MASK) : (its_ctrl | GITS_CTLR_ENABLE);
    io_reg_write32(base_addr, GITS_CTLR, its_ctrl);
}

void its_set_enable(void *base_addr)
{
    its_set_enable_disable(base_addr, true);
}

void its_set_disable(void *base_addr)
{
    its_set_enable_disable(base_addr, false);
}

static U64 do_its_alloc_its_table(struct its_device_s *its_device,
    U64 baser_type, U64 entry_size, U64 page_real_size, U64 *size)
{
    U64 table_size = 0ULL;
    U64 phy_addr = 0ULL;

    switch (baser_type) {
        case GITS_BASER_TYPE_DEVICE:
            table_size = its_alloc_table_mem(GITS_COLLECTION_ID_WIDTH,
                entry_size, page_real_size, &its_device->its_res.dt_addr);
            phy_addr = ptr_to_U64(its_device->its_res.dt_addr);
            break;
        case GITS_BASER_TYPE_IRQ_COLLECT:
            table_size = its_alloc_table_mem(GITS_COLLECTION_ID_WIDTH,
                entry_size, page_real_size, &its_device->its_res.ct_addr);
            phy_addr = ptr_to_U64(its_device->its_res.ct_addr);
            break;
        default:
            break;
    }

    if (table_size ==  0ULL) {
        phy_addr = 0ULL;
    }

    *size = table_size;
    return phy_addr;
}

static U32 its_get_page_size(U32 page_size)
{
    U32 size;
    switch (page_size) {
        case GITS_BASER_PAGE_SZ_4K:
            size = 0x1000U;
            break;
        case GITS_BASER_PAGE_SZ_16K:
            size = 0x4000U;
            break;
        case GITS_BASER_PAGE_SZ_64K:
            size = 0x10000U;
            break;
        default:
            size = 0x10000U;
            break;        
    }

    return size;
}

int its_alloc_its_tables(struct its_device_s *its_device)
{
    for (U32 i = 0U; i < GITS_BASER_REG_NR; i++) {
        U64 its_baser = io_reg_read64(its_device->base_addr, GITS_BASER + i * sizeof(U64));

        U64 page_size = GITS_BASER_PAGE_SIZE(its_baser);
        page_size = (page_size == GITS_BASER_PAGE_SZ_RESERVE) ? GITS_BASER_PAGE_SZ_64K : page_size;
        U64 page_real_size = its_get_page_size(page_size);
        U64 table_size;
        U64 phy_addr = 0ULL;
        phy_addr = do_its_alloc_its_table(its_device, GITS_BASER_TYPE(its_baser),
            GITS_BASER_ENTRY_SIZE(its_baser), page_real_size, &table_size);
        if (phy_addr == 0ULL) {
            continue;
        }

        if (page_size == GITS_BASER_PAGE_SZ_64K) {
            phy_addr = (((phy_addr >> 48U) & 0xFU) << 12U) | (phy_addr & ~0xFFFFUL);
        }

        its_baser = GITS_BASER_VALID |
            (GITS_CACHE_RaWaWb << GITS_CBASER_IN_CACHE_SHIFT) |
            (GITS_BASER_TYPE(its_baser) << GITS_BASER_TYPE_SHIFT) |
            ((GITS_BASER_ENTRY_SIZE(its_baser) - 1U) << GITS_BASER_ENTRY_SIZE_SHIFT) |
            (phy_addr & GITS_BASER_PHY_ADDR_MASK) |
            (GITS_BASER_InnerShareable << GITS_BASER_SHAREABILITY_SHIFT) |
            (page_size << GITS_BASER_PAGE_SIZE_SHIFT) |
            ((table_size / page_real_size - 1U) << GITS_BASER_PAGE_NR_SHIFT);
        
        io_reg_write64(its_device->base_addr, GITS_BASER + i * sizeof(U64), its_baser);
        U64 tmp_value = io_reg_read64(its_device->base_addr, GITS_BASER + i * sizeof(U64));
        if (tmp_value != its_baser) {
            PRT_Printf("[ERROE] its baser register write failed.\n");
            return OS_ERROR;
        }

        PRT_Printf("ITS: config its table, 0x%llx table type, value:0x%llx.\n", GITS_BASER_TYPE(its_baser), its_baser);
    }
    PRT_Printf("ITS: gic_its finish alloc table.\n");

    return OS_OK;
}

int its_init_cmd_queue(struct its_device_s *its_device)
{
    its_device->its_res.cmdq_base = its_alloc_cmd_queue();
    if (its_device->its_res.cmdq_base == NULL) {
        return OS_ERROR;
    }

    U64 its_cbaser = io_reg_read64(its_device->base_addr, GITS_CBASER);
    U64 cache_ability = GITS_CBASER_IN_CACHE(its_cbaser);
    U64 phy_addr = ptr_to_U64(its_device->its_res.cmdq_base);  
    U64 page_nr = GITS_CMD_QUEUE_SIZE / 4096U;

    its_cbaser = GITS_CBASER_VALID | (cache_ability << GITS_CBASER_IN_CACHE_SHIFT) |
        (phy_addr & GITS_CBASER_PHYADDR_MASK) |
        (GITS_SHARE_InnerShareable << GITS_CBASER_SHAREABILITY_SHIFT) |
        (page_nr - 1U);
    io_reg_write64(its_device->base_addr, GITS_CBASER, its_cbaser);

    U64 tmp_value = io_reg_read64(its_device->base_addr, GITS_CBASER);
    if (tmp_value != its_cbaser) {
        PRT_Printf("[ERROE] its cbaser register write failed.\n");
        return OS_ERROR;
    }
    
    PRT_Printf("ITS: the GITS_CBASER register is configured as :0x%llx.\n", its_cbaser);

    io_reg_write64(its_device->base_addr, GITS_CWRITE, 0UL);

    return OS_OK;
}

int its_alloc_lpi_prop_table(struct its_device_s *its_device)
{
    void *prop_table_addr = boot_alloc_align(GITS_PROP_TABLE_SIZE, MEM_ADDR_ALIGN_4K);
    if (prop_table_addr == NULL) {
        return OS_ERROR;
    }

    for (U64 i = 0; i < GITS_PROP_TABLE_SIZE; i++) {
        *U64_to_ptr(ptr_to_U64(prop_table_addr) + i * sizeof(U8), U8) = GITS_LPI_DEFAULT_PRIO | GITS_LPI_PROP_GROUP1;
    }

    its_device->its_res.prop_table_addr = prop_table_addr;

    return OS_OK;
}

int its_alloc_lpi_pend_table(struct its_device_s *its_device)
{
    for (U32 i = 0; i < OS_MAX_CORE_NUM; i++) {
        its_device->its_res.pending_table_addr[i] =
            boot_alloc_align(GITS_PEND_TABLE_SIZE, MEM_ADDR_ALIGN_4K);
        if (its_device->its_res.pending_table_addr[i] == NULL) {
            PRT_Printf("[ERROR] its alloc pend table memory failed.\n");
            return OS_ERROR;
        }
        mem_zero_b(its_device->its_res.pending_table_addr[i], GITS_PEND_TABLE_SIZE);
    }

    return OS_OK;
}

static void its_set_irq_enable_disable(struct its_device_s *its_device, U32 hwirq, bool enable)
{
    U8 *prop_table_entry = (U8 *)(its_device->its_res.prop_table_addr) + (hwirq - GITS_LPI_HWIRQ_MIN);

    if (enable) {
        *prop_table_entry = *prop_table_entry | GITS_LPI_PROP_ENABLE;
    } else {
        *prop_table_entry = *prop_table_entry & ~GITS_LPI_PROP_ENABLE_MASK;
    }

    struct its_cmd_invall_arg_s cmd_arg;
    mem_zero_s(cmd_arg);
    cmd_arg.icid = 0ULL;
    its_send_cmd(its_device, its_cmd_invall, (union its_cmd_arg_s *)&cmd_arg);
}

static int its_check_validate(struct its_device_s *its_device, U32 hwirq)
{
    if (its_device == NULL) {
        PRT_Printf("ITS: its device is NULL.\n");
        return OS_ERROR;
    } else if (its_device->its_res.prop_table_addr == NULL) {
        PRT_Printf("ITS: prop_table_addr is NULL.\n");
        return OS_ERROR;
    } else if (hwirq >= its_device->hwirq_max) {
        PRT_Printf("ITS: hwirq:%u is too large.\n", hwirq);
        return OS_ERROR;
    }

    return OS_OK;
}

int its_set_irq_enable(struct its_device_s *its_device, U32 hwirq)
{
    int ret = its_check_validate(its_device, hwirq);
    if (ret == OS_OK) {
        its_set_irq_enable_disable(its_device, hwirq, true);
    }

    return ret;
}

int its_set_irq_disable(struct its_device_s *its_device, U32 hwirq)
{
    int ret = its_check_validate(its_device, hwirq);
    if (ret == OS_OK) {
        its_set_irq_enable_disable(its_device, hwirq, false);
    }

    return ret;
}

static void its_set_clear_irq_pending(struct its_device_s *its_device, U32 hwirq, bool set)
{
    struct its_device_info_s *device_info = NULL;
    struct its_device_info_s *posion = NULL;
    list_for_every_entry(&its_device->dev_info_head, posion, struct its_device_info_s, list) {
        if ((posion->hwirq_base <= hwirq) && (hwirq < (posion->hwirq_base + posion->event_nr))) {
            device_info = posion;
            break;
        }
    }

    if (device_info != NULL) {
        union its_cmd_arg_s cmd_arg;
        mem_zero_s(cmd_arg);
        cmd_arg.cmd_int.device_id = device_info->device_id;
        cmd_arg.cmd_int.event_id = hwirq - device_info->hwirq_base;
        if (set) {
            its_send_cmd(its_device, its_cmd_int, &cmd_arg);
        } else {
            its_send_cmd(its_device, its_cmd_clear, &cmd_arg);
        }
    }
}

int its_set_irq_pending(struct its_device_s *its_device, U32 hwirq)
{
    int ret = its_check_validate(its_device, hwirq);
    if (ret == OS_OK) {
        its_set_clear_irq_pending(its_device, hwirq, true);
    }

    return ret;
}

int its_clear_irq_pending(struct its_device_s *its_device, U32 hwirq)
{
    int ret = its_check_validate(its_device, hwirq);
    if (ret == OS_OK) {
        its_set_clear_irq_pending(its_device, hwirq, false);
    }
    
    return ret;
}

static void its_init_device_info(struct its_device_info_s *device_info, U32 device_id, U32 nr, U32 hwirq_base)
{
    mem_zero_s(*device_info);

    device_info->device_id = device_id;
    device_info->event_nr = nr;
    device_info->hwirq_base = hwirq_base;
    list_initialize(&device_info->list);
}

static void its_config_pend_prop_table(struct redist_addr_info_s *gicr_addr, unsigned int cpu,
    struct its_device_s *its_device)
{
    U64 gicr_pendbaser;
    U64 gicr_propbaser;

    gicr_pendbaser = ((ptr_to_U64(its_device->its_res.pending_table_addr[cpu])) |
        (GITS_SHARE_InnerShareable << GICV3_GICR_PENDBASER_SHAREABILITY_SHIFT) |
        (GITS_CACHE_RaWaWb << GICV3_GICR_PENDBASER_CACHEABILITY_SHIFT));

    io_reg_write64(U64_to_ptr(gicr_addr->phy_addr, void *), GICV3_GICR_PENDBASER, gicr_pendbaser);

    gicr_propbaser = ((ptr_to_U64(its_device->its_res.prop_table_addr)) |
        (GITS_SHARE_InnerShareable << GICV3_GICR_PROPBASER_SHAREABILITY_SHIFT) |
        (GITS_CACHE_RaWaWb << GICV3_GICR_PROPBASER_CACHEABILITY_SHIFT) |
        GICV3_GICR_PROPBASER_IDBITS);

    io_reg_write64(U64_to_ptr(gicr_addr->phy_addr, void *), GICV3_GICR_PROPBASER, gicr_propbaser);
}

/*get its config attribute*/
void its_get_attr_info(struct its_device_s *its_device)
{
    U32 its_type = io_reg_read32(its_device->base_addr, GITS_TYPER);
    its_device->its_attr.device_id_bit = GITS_TYPER_DEVID_BITS(its_type);
    its_device->its_attr.itt_entry_size = GITS_TYPER_ITE_SIZE(its_type);
    its_device->its_attr.pta = GITS_TYPER_PTA_BITS(its_type);
    its_device->its_attr.virtual = GITS_TYPER_ITE_VIRTUAL(its_type);
}

int its_alloc_memory(struct its_device_s *its_device)
{
    if (its_alloc_its_tables(its_device) != OS_OK) {
        return OS_ERROR;
    }

    if (its_init_cmd_queue(its_device) != OS_OK) {
        return OS_ERROR;
    }

    if (its_alloc_lpi_pend_table(its_device) != OS_OK) {
        return OS_ERROR;
    }

    if (its_alloc_lpi_prop_table(its_device) != OS_OK) {
        return OS_ERROR;
    }

    return OS_OK;
}

static void its_set_lpi_enable_disable(struct redist_addr_info_s *gicr_addr, bool enable)
{
    U32 gicr_ctrl = io_reg_read32(U64_to_ptr(gicr_addr->phy_addr, void *), GITS_GICR_CTLR);

    if (enable) {
        gicr_ctrl |= GITS_GICR_CTLR_ENABLE_LPI;
    } else {
        gicr_ctrl &= ~GITS_GICR_CTLR_ENABLE_LPI;
    }

    io_reg_write32(U64_to_ptr(gicr_addr->phy_addr, void *), GITS_GICR_CTLR, gicr_ctrl);

    dsb(sy);
}

static inline void its_gicr_set_lpi_enable(struct redist_addr_info_s *gicr_addr)
{
    its_set_lpi_enable_disable(gicr_addr, true);
}

static inline void its_gicr_set_lpi_disable(struct redist_addr_info_s *gicr_addr)
{
    its_set_lpi_enable_disable(gicr_addr, false);
}

void its_init_gicr(struct its_device_s *its_device, struct redist_addr_info_s *gicr_addr, U32 cpu_id)
{
    its_gicr_set_lpi_disable(gicr_addr);
    its_config_pend_prop_table(gicr_addr, cpu_id, its_device);
    its_gicr_set_lpi_enable(gicr_addr);
}

void its_device_init(struct its_device_s *its_device)
{
    its_device->base_addr = U64_to_ptr(GITS_BASE_ADDR, void *);
    its_device->hwirq_max = GITS_MAX_HWIRQ;

    its_set_disable(its_device->base_addr);

    its_get_attr_info(its_device);

    struct redist_addr_info_s gicr_addr = {0};
    gicr_addr.phy_addr = GICR_BASE0;

    its_alloc_memory(its_device);

    its_init_gicr(its_device, &gicr_addr, OsGetCoreID());

    its_set_enable(its_device->base_addr);
}

void its_device_info_init(struct its_device_s *its_device, struct its_device_info_s *device_list, U32 device_num)
{
    list_initialize(&its_device->dev_info_head);
    for (U32 i = 0; i < device_num; i++) {
        struct its_device_info_s *device_info =
            (struct its_device_info_s *)PRT_MemAlloc(0, OS_MEM_DEFAULT_FSC_PT, sizeof(struct its_device_info_s));

        its_init_device_info(device_info, device_list[i].device_id, device_list[i].event_nr, device_list[i].hwirq_base);
        list_add_tail(&its_device->dev_info_head, &device_info->list);
    }
}

void its_map_single_device_info(struct its_device_s *its_device, struct its_device_info_s *device_info, U32 collect_id)
{
    its_map_device_id_to_itt(its_device, device_info->device_id, device_info->event_nr);
    
    U32 size = __fls(device_info->event_nr);
    size = (size == 0U) ? 1U : size;
    U32 max_event_id = (1U << size);

    for (U32 event_id = 0; event_id < max_event_id; event_id++) {
        U32 hwirq = device_info->hwirq_base + event_id;
        its_config_lpi_route(its_device, device_info->device_id, event_id, hwirq, collect_id);
    }
}

void its_setup_device_table(struct its_device_s *its_device, U32 collect_id)
{
    struct its_device_info_s *device_info = NULL;
    list_for_every_entry(&its_device->dev_info_head, device_info, struct its_device_info_s, list) {
        if (device_info != NULL) {
            its_map_single_device_info(its_device, device_info, collect_id);
        }
    }
}