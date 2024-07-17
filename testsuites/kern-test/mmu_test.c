#include "prt_config.h"
#include "prt_mem.h"
#include "prt_log.h"
#include "securec.h"
#include "time.h"
#include "mmu.h"
#include "cpu_config.h"
#include "kern_test_public.h"

extern U32 data_copy_start;
extern U32 __text_start;
extern U32 __text_end;

static U32 mmu_level2shift(U32 level, U32 granule)
{
    if (granule == MMU_GRANULE_4K) {
        return (U32)(MMU_BITS_12 + MMU_BITS_9 * (MMU_LEVEL_3 - level));
    } else {
        return (U32)(MMU_BITS_16 + MMU_BITS_13 * (MMU_LEVEL_3 - level));
    }
}

// 只适用于armv8, 查询mmu表
static U64 test_mmu_find_pte_content(U64 addr)
{
    U64 *pte = NULL;
    U64 entryContent;
    U64 idx;
    U32 i, granule, va_bits, start_level;
    U64 table_addr, tcr;

    PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "try to find mmu pte for addr:%llx", addr);

    OS_EMBED_ASM("mrs %0, ttbr0_el1" : "=r"(table_addr) : : "memory", "cc");
    OS_EMBED_ASM("mrs %0, tcr_el1" : "=r"(tcr) : : "memory", "cc");

    if ((TCR_TG0_64K & tcr) == TCR_TG0_64K) {
        granule = MMU_GRANULE_64K;
    } else {
        granule = MMU_GRANULE_4K;
    }
    va_bits = 64 - (tcr & 0x3F);
    if (granule == MMU_GRANULE_4K) {
        if (va_bits < MMU_BITS_39) {
            start_level = MMU_LEVEL_1;
        } else {
            start_level = MMU_LEVEL_0;
        }
    } else {
        if (va_bits <= MMU_BITS_36) {
            start_level = MMU_LEVEL_2;
        } else {
            start_level = MMU_LEVEL_1;
        }
    }

    pte = (U64 *)table_addr;
    PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "mmu tbl addr:%llx, granule:%u, level:%u", table_addr, granule, start_level);

    for (i = start_level; i < MMU_LEVEL_MAX; ++i) {
        PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "mmu level %u table addr:%llx", i, (U64)pte);
        if (granule == MMU_GRANULE_4K) {
            idx = (addr >> mmu_level2shift(i, granule)) & 0x1FF;
        } else {
            idx = (addr >> mmu_level2shift(i, granule)) & 0x1FFF;
        }

        pte += idx;
        entryContent = *pte;

        PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "pte idx:%llx, addr:%llx, content:%llx", idx, (U64)pte, entryContent);

        if (i == MMU_LEVEL_3) {
            return *pte;
        }

        if (((U32)(*pte & PTE_TYPE_MASK)) != PTE_TYPE_TABLE) {
            return *pte;
        }

        if (granule == MMU_GRANULE_4K) {
            pte = (U64 *)(*pte & PTE_TABLE_ADDR_MARK_4K);
        } else {
            pte = (U64 *)(*pte & PTE_TABLE_ADDR_MARK_64K);
        }
    }
    return 0;
}

// 给定地址, 查mmu表找到pte, 检查pte可读可写属性, 代码段地址不可写, 其他地址可写
static int test_mmu_check_pte_attr(void)
{
    U64 pteEntry;
    pteEntry = test_mmu_find_pte_content(MMU_OPENAMP_ADDR);
    TEST_IF_ERR_RET(((pteEntry & PTE_TYPE_VALID) == 0), "[mmu_check_pte_attr] error: entry invalid");
    TEST_IF_ERR_RET(((pteEntry & PTE_BLOCK_AP_R) != 0),
        "[mmu_check_pte_attr] error: openamp addr shoud be rw, but pte found ro");
    TEST_LOG("[mmu_check_pte_attr] check openamp addr rw success");

    pteEntry = test_mmu_find_pte_content(MMU_UART_ADDR);
    TEST_IF_ERR_RET(((pteEntry & PTE_TYPE_VALID) == 0), "[mmu_check_pte_attr] error: entry invalid");
    TEST_IF_ERR_RET(((pteEntry & PTE_BLOCK_AP_R) != 0),
        "[mmu_check_pte_attr] error: uart addr shoud be rw, but pte found ro");
    TEST_LOG("[mmu_check_pte_attr] check uart addr rw success");

    pteEntry = test_mmu_find_pte_content((U64)&__text_start);
    TEST_IF_ERR_RET(((pteEntry & PTE_TYPE_VALID) == 0), "[mmu_check_pte_attr] error: entry invalid");
    TEST_IF_ERR_RET(((pteEntry & PTE_BLOCK_AP_R) == 0),
        "[mmu_check_pte_attr] error: text start addr shoud be ro, but pte found rw");
    TEST_LOG("[mmu_check_pte_attr] check text start addr ro success");

    pteEntry = test_mmu_find_pte_content((U64)&__text_end);
    TEST_IF_ERR_RET(((pteEntry & PTE_TYPE_VALID) == 0), "[mmu_check_pte_attr] error: entry invalid");
    TEST_IF_ERR_RET(((pteEntry & PTE_BLOCK_AP_R) == 0),
        "[mmu_check_pte_attr] error: text end addr shoud be ro, but pte found rw");
    TEST_LOG("[mmu_check_pte_attr] check text end addr ro success");

    pteEntry = test_mmu_find_pte_content((U64)&data_copy_start);
    TEST_IF_ERR_RET(((pteEntry & PTE_TYPE_VALID) == 0), "[mmu_check_pte_attr] error: entry invalid");
    TEST_IF_ERR_RET(((pteEntry & PTE_BLOCK_AP_R) != 0),
        "[mmu_check_pte_attr] error: data_copy_start shoud be rw, but pte found ro");
    TEST_LOG("[mmu_check_pte_attr] check data_copy_start rw success");

    pteEntry = test_mmu_find_pte_content((U64)&data_copy_start - 1);
    TEST_IF_ERR_RET(((pteEntry & PTE_TYPE_VALID) == 0), "[mmu_check_pte_attr] error: entry invalid");
    TEST_IF_ERR_RET(((pteEntry & PTE_BLOCK_AP_R) == 0),
        "[mmu_check_pte_attr] error: data_copy_start - 1 shoud be ro, but pte found rw");
    TEST_LOG("[mmu_check_pte_attr] check data_copy_start - 1 ro success");

    return 0;
}

static int test_mmu_triggr_exception(void)
{
    U32 *data_copy_addr = ((U32 *)&data_copy_start);
    U32 *text_start_addr = ((U32 *)&__text_start) + 10;
    U32 *text_end_addr = ((U32 *)&__text_end) - 10;

    TEST_LOG("[mmu_triggr_exception] read addr, should success");
    TEST_LOG_FMT("[mmu_triggr_exception] text start addr:%llx, content:%x", (U64)text_start_addr, *text_start_addr);
    TEST_LOG_FMT("[mmu_triggr_exception] text end addr:%llx, content:%x", (U64)text_end_addr, *text_end_addr);
    TEST_LOG_FMT("[mmu_triggr_exception] data copy addr:%llx, content:%x", (U64)data_copy_addr, *data_copy_addr);

    TEST_LOG("[mmu_triggr_exception] write data copy addr, should success");
    *data_copy_addr = 0xABAB;
    TEST_IF_ERR_RET((*data_copy_addr != 0xABAB), "[mmu_triggr_exception] error: data write wrong");
    TEST_LOG_FMT("[mmu_triggr_exception] data copy write success, content:%x", *data_copy_addr);

    TEST_LOG("[mmu_triggr_exception] write test start addr, should fail and trigger exception");
    *text_start_addr = 0xABAB;
    TEST_LOG_FMT("[mmu_triggr_exception] test start write success, content:%x", *text_start_addr);
    TEST_LOG("[mmu_triggr_exception] test fail!");
    return 1;
}

test_case_t g_cases[] = {
    TEST_CASE_Y(test_mmu_check_pte_attr),
    TEST_CASE_N(test_mmu_triggr_exception),
};

int g_test_case_size = sizeof(g_cases);

void prt_kern_test_end()
{
    TEST_LOG("MMU check test finished\n");
}
