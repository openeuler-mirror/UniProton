#ifndef _CONFORMANCE_RUN_PRT_MEM_TEST_H
#define _CONFORMANCE_RUN_PRT_MEM_TEST_H

/*
 * PRT_Mem* 直接接口测试（common：两算法都能通过）。
 * 底层算法由 defconfig 选定（FSC / TLSF），本组用例只经标准 PRT_MemAlloc/
 * PRT_MemFree/PRT_MemAllocAlign 接口验证分配/释放/对齐/复用等功能，不依赖
 * 任何算法私有布局。算法独有用例（如 FSC 的 tail magic）不在此处登记。
 */
extern int prt_mem_001(void);
extern int prt_mem_002(void);
extern int prt_mem_003(void);
extern int prt_mem_004(void);
extern int prt_mem_005(void);
extern int prt_mem_006(void);
extern int prt_mem_007(void);
extern int prt_mem_008(void);
extern int prt_mem_009(void);
extern int prt_mem_010(void);

typedef int test_run_main(void);

test_run_main *run_test_arry_prt[] = {
    prt_mem_001,
    prt_mem_002,
    prt_mem_003,
    prt_mem_004,
    prt_mem_005,
    prt_mem_006,
    prt_mem_007,
    prt_mem_008,
    prt_mem_009,
    prt_mem_010,
};

char run_test_name_prt[][50] = {
    "prt_mem_001",
    "prt_mem_002",
    "prt_mem_003",
    "prt_mem_004",
    "prt_mem_005",
    "prt_mem_006",
    "prt_mem_007",
    "prt_mem_008",
    "prt_mem_009",
    "prt_mem_010",
};

#endif /* _CONFORMANCE_RUN_PRT_MEM_TEST_H */
