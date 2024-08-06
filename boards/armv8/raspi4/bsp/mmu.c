#include "prt_buildef.h"
#include "prt_typedef.h"
#include "prt_module.h"
#include "prt_errno.h"
#include "mmu.h"
#include "cache_asm.h"
#include "prt_sys.h"
#include "prt_task.h"
#include "cpu_config.h"

extern U64 g_mmu_page_begin;
extern U64 g_mmu_page_end;

static mmu_mmap_region_s g_mem_map_info[] = {
    {
        .virt      = MMU_OPENAMP_ADDR,
        .phys      = MMU_OPENAMP_ADDR,
        .size      = OPENAMP_SHM_SIZE,
        .max_level = 0x2,
        .attrs     = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    }, {
        .virt      = MMU_IMAGE_ADDR,
        .phys      = MMU_IMAGE_ADDR,
        .size      = 0x1000000,
        .max_level = 0x2,
        .attrs     = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
    }, {
        .virt      = MMU_GIC_ADDR,
        .phys      = MMU_GIC_ADDR,
        .size      = 0x1000000,
        .max_level = 0x2,
        .attrs     = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    }, {    
        .virt      = MMU_UART_ADDR,
        .phys      = MMU_UART_ADDR,
        .size      = 0x2000,
        .max_level = 0x2,
        .attrs     = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    }
};

static mmu_ctrl_s g_mmu_ctrl = { 0 };

static U64 mmu_get_tcr(U32 *pips, U32 *pva_bits)
{
    U64 max_addr = 0;
    U64 ips, va_bits;
    U64 tcr;
    U32 i;
    U32 mmu_table_num = sizeof(g_mem_map_info) / sizeof(mmu_mmap_region_s);
    
    for (i = 0; i < mmu_table_num; ++i) {
        max_addr = MAX(max_addr, g_mem_map_info[i].virt + g_mem_map_info[i].size);
    }
    
    if (max_addr > (1ULL << MMU_BITS_44)) {
        ips = MMU_PHY_ADDR_LEVEL_5;
        va_bits = MMU_BITS_48;
    } else if (max_addr > (1ULL << MMU_BITS_42)) {
        ips = MMU_PHY_ADDR_LEVEL_4;
        va_bits = MMU_BITS_44;
    } else if (max_addr > (1ULL << MMU_BITS_40)) {
        ips = MMU_PHY_ADDR_LEVEL_3;
        va_bits = MMU_BITS_42;
    } else if (max_addr > (1ULL << MMU_BITS_36)) {
        ips = MMU_PHY_ADDR_LEVEL_2;
        va_bits = MMU_BITS_40;
    } else if (max_addr > (1ULL << MMU_BITS_32)) {
        ips = MMU_PHY_ADDR_LEVEL_1;
        va_bits = MMU_BITS_36;
    } else {
        ips = MMU_PHY_ADDR_LEVEL_0;
        va_bits = MMU_BITS_32;
    }
    
    tcr = TCR_EL1_RSVD | TCR_IPS(ips);
    
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        tcr |= TCR_TG0_4K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;
    } else {
        tcr |= TCR_TG0_64K | TCR_SHARED_INNER | TCR_ORGN_WBWA | TCR_IRGN_WBWA;
    }
    
    tcr |= TCR_T0SZ(va_bits);
    
    if (pips != NULL) {
        *pips = ips;
    }
    
    if (pva_bits != NULL) {
        *pva_bits = va_bits;
    }
    
    return tcr;
}

static U32 mmu_get_pte_type(U64 const *pte)
{
    return (U32)(*pte & PTE_TYPE_MASK);
}

static U32 mmu_level2shift(U32 level)
{
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        return (U32)(MMU_BITS_12 + MMU_BITS_9 * (MMU_LEVEL_3 - level));
    } else {
        return (U32)(MMU_BITS_16 + MMU_BITS_13 * (MMU_LEVEL_3 - level));
    }
}

static U64 *mmu_find_pte(U64 addr, U32 level)
{
    U64 *pte = NULL;
    U64 idx;
    U32 i;
    
    if (level < g_mmu_ctrl.start_level) {
        return NULL;
    }
    
    pte = (U64 *)g_mmu_ctrl.tlb_addr;
    
    for (i = g_mmu_ctrl.start_level; i < MMU_LEVEL_MAX; ++i) {
        if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
            idx = (addr >> mmu_level2shift(i)) & 0x1FF;
        } else {
            idx = (addr >> mmu_level2shift(i)) & 0x1FFF;
        }
        
        pte += idx;
        
        if (i == level) {
            return pte;
        }
        
        if (mmu_get_pte_type(pte) != PTE_TYPE_TABLE) {
            return NULL;
        }
        
        if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
            pte = (U64 *)(*pte & PTE_TABLE_ADDR_MARK_4K);
        } else {
            pte = (U64 *)(*pte & PTE_TABLE_ADDR_MARK_64K);
        }
    }
    
    return NULL;
}

static U64 *mmu_create_table(void)
{
    U32 pt_len;
    U64 *new_table = (U64 *)g_mmu_ctrl.tlb_fillptr;
    
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        pt_len = MAX_PTE_ENTRIES_4K * sizeof(U64);
    } else {
        pt_len = MAX_PTE_ENTRIES_64K * sizeof(U64);
    }
    
    g_mmu_ctrl.tlb_fillptr += pt_len;
    
    if (g_mmu_ctrl.tlb_fillptr - g_mmu_ctrl.tlb_addr > g_mmu_ctrl.tlb_size) {
        return NULL;
    }
    
    (void)memset_s((void *)new_table, MAX_PTE_ENTRIES_64K * sizeof(U64), 0, pt_len);
    
    return new_table;
}

static void mmu_set_pte_table(U64 *pte, U64 *table)
{
    *pte = PTE_TYPE_TABLE | (U64)table;
}

static S32 mmu_add_map_pte_process(mmu_mmap_region_s const *map, U64 *pte, U64 phys, U32 level)
{
    U64 *new_table = NULL;
    
    if (level < map->max_level) {
        if (mmu_get_pte_type(pte) == PTE_TYPE_FAULT) {
            new_table = mmu_create_table();
            if (new_table == NULL) {
                return -1;
            }
            mmu_set_pte_table(pte, new_table);
        }
    } else if (level == MMU_LEVEL_3) {
        *pte = phys | map->attrs | PTE_TYPE_PAGE;
    } else {
        *pte = phys | map->attrs | PTE_TYPE_BLOCK;
    }
    
    return 0;
}

static S32 mmu_add_map(mmu_mmap_region_s const *map)
{
    U64 virt = map->virt;
    U64 phys = map->phys;
    U64 max_level = map->max_level;
    U64 start_level = g_mmu_ctrl.start_level;
    U64 block_size = 0;
    U64 map_size = 0;
    U32 level;
    U64 *pte = NULL;
    S32 ret;
    
    if (map->max_level <= start_level) {
        return -2;
    }
    
    while (map_size < map->size) {
        for (level = start_level; level <= max_level; ++level) {
            pte = mmu_find_pte(virt, level);
            if (pte == NULL) {
                return -3;
            }
            
            ret = mmu_add_map_pte_process(map, pte, phys, level);
            if (ret) {
                return ret;
            }
            
            if (level != start_level) {
                block_size = 1ULL << mmu_level2shift(level);
            }
        }
        
        virt += block_size;
        phys += block_size;
        map_size += block_size;
    }
    
    return 0;
}

static inline void mmu_set_ttbr_tcr_mair(U64 table, U64 tcr, U64 attr)
{
    OS_EMBED_ASM("dsb sy");
    
    OS_EMBED_ASM("msr ttbr0_el1, %0" : : "r" (table) : "memory");
    OS_EMBED_ASM("msr ttbr1_el1, %0" : : "r" (table) : "memory");
    OS_EMBED_ASM("msr tcr_el1, %0" : : "r" (tcr) : "memory");
    OS_EMBED_ASM("msr mair_el1, %0" : : "r" (attr) : "memory");
    
    OS_EMBED_ASM("isb");
}

static U32 mmu_setup_pgtables(mmu_mmap_region_s *mem_map, U32 mem_region_num, U64 tlb_addr, U64 tlb_len, U32 granule)
{
    U32 i;
    U32 ret;
    U64 tcr;
    U64 *new_table = NULL;
    
    g_mmu_ctrl.tlb_addr = tlb_addr;
    g_mmu_ctrl.tlb_size = tlb_len;
    g_mmu_ctrl.tlb_fillptr = tlb_addr;
    g_mmu_ctrl.granule = granule;
    g_mmu_ctrl.start_level = 0;
    
    tcr = mmu_get_tcr(NULL, &g_mmu_ctrl.va_bits);
    
    if (g_mmu_ctrl.granule == MMU_GRANULE_4K) {
        if (g_mmu_ctrl.va_bits < MMU_BITS_39) {
            g_mmu_ctrl.start_level = MMU_LEVEL_1;
        } else {
            g_mmu_ctrl.start_level = MMU_LEVEL_0;
        }
    } else {
        if (g_mmu_ctrl.va_bits <= MMU_BITS_36) {
            g_mmu_ctrl.start_level = MMU_LEVEL_2;
        } else {
            g_mmu_ctrl.start_level = MMU_LEVEL_1;
            return 3;
        }
    }
    
    new_table = mmu_create_table();
    if (new_table == NULL) {
        return 1;
    }
    
    for (i = 0; i < mem_region_num; ++i) {
        ret = mmu_add_map(&mem_map[i]);
        if (ret) {
            return ret;
        }
    }
    
    mmu_set_ttbr_tcr_mair(g_mmu_ctrl.tlb_addr, tcr, MEMORY_ATTRIBUTES);
    
    return 0;
}

static S32 mmu_setup(void)
{
    S32 ret;
    U64 page_addr;
    U64 page_len;
    
    page_addr = (U64)&g_mmu_page_begin;
    page_len = (U64)&g_mmu_page_end - (U64)&g_mmu_page_begin;
    
    ret = mmu_setup_pgtables(g_mem_map_info, (sizeof(g_mem_map_info) / sizeof(mmu_mmap_region_s)),
                             page_addr, page_len, MMU_GRANULE_4K);
    if (ret) {
        return ret;
    }
    
    return 0;
}

S32 mmu_init(void)
{
    S32 ret;

    ret = mmu_setup();
    if (ret) {
        return ret;
    }

    os_asm_invalidate_dcache_all();
    os_asm_invalidate_icache_all();
    os_asm_invalidate_tlb_all();

    set_sctlr(get_sctlr() | CR_C | CR_M | CR_I);

    return 0;
}
