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
 * Create: 2024-03-15
 * Description: Perf
 */

#ifndef ARMV8_PMU_H
#define ARMV8_PMU_H

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define PRT_PMU_IRQ_NR                23

#define ARMV8_FLAG_MASK               0xffffffff         /* Mask for writable bits */
#define ARMV8_OVERFLOWED_MASK         ARMV8_FLAG_MASK    /* Mask for pmu overflow */
#define ARMV8_EVTYPE_MASK             0xc800ffff         /* Mask for writable bits */
#define ARMV8_READ_MASK               0x00000000ffffffff /* Mask for read count */
#define ARMV8_PERIOD_MASK             0xffffffff00000000 /* Mask for period set */

#define ARMV8_PMCR_E                  (1U << 0) /* Enable all counters */
#define ARMV8_PMCR_P                  (1U << 1) /* Reset all counters */
#define ARMV8_PMCR_C                  (1U << 2) /* Cycle counter reset */
#define ARMV8_PMCR_D                  (1U << 3) /* CCNT counts every 64th cpu cycle */
#define ARMV8_PMCR_X                  (1U << 4) /* Export to ETM */
#define ARMV8_PMCR_DP                 (1U << 5) /* Disable CCNT if non-invasive debug */
#define ARMV8_PMCR_LC                 (1U << 6) /* Overflow on 64 bit cycle counter */
#define ARMV8_PMCR_MASK               0x7f      /* Mask for writable bits */

#define ARMV8_MAX_COUNTERS            32
#define ARMV8_COUNTER_MASK            (ARMV8_MAX_COUNTERS - 1)

#define ARMV8_IDX_MAX_COUNTER         7
#define ARMV8_IDX_CYCLE_COUNTER       0
#define ARMV8_IDX_COUNTER0            1

#define ARMV8_IDX2CNT(x)              (((x) - ARMV8_IDX_COUNTER0) & ARMV8_COUNTER_MASK)
#define ARMV8_CNT2BIT(x)              (1ULL << (x))

enum PmuEventType {
    ARMV8_PMUV3_PERF_HW_PMNC_SW_INCR               = 0x00,
    ARMV8_PMUV3_PERF_HW_L1_ICACHE_REFILL           = 0x01, /* icache access miss event */
    ARMV8_PMUV3_PERF_HW_ITLB_REFILL                = 0x02,
    ARMV8_PMUV3_PERF_HW_L1_DCACHE_REFILL           = 0x03, /* dcache access miss event */
    ARMV8_PMUV3_PERF_HW_L1_DCACHE_ACCESS           = 0x04, /* dcache access event */
    ARMV8_PMUV3_PERF_HW_DTLB_REFILL                = 0x05,
    ARMV8_PMUV3_PERF_HW_MEM_READ                   = 0x06,
    ARMV8_PMUV3_PERF_HW_MEM_WRITE                  = 0x07,
    ARMV8_PMUV3_PERF_HW_INSTR_EXECUTED             = 0x08, /* instructions event */
    ARMV8_PMUV3_PERF_HW_EXC_TAKEN                  = 0x09,
    ARMV8_PMUV3_PERF_HW_EXC_EXECUTED               = 0x0A,
    ARMV8_PMUV3_PERF_HW_CID_WRITE                  = 0x0B,
    ARMV8_PMUV3_PERF_HW_PC_WRITE                   = 0x0C, /* branch forecast event */
    ARMV8_PMUV3_PERF_HW_PC_IMM_BRANCH              = 0x0D,
    ARMV8_PMUV3_PERF_HW_PC_PROC_RETURN             = 0x0E,
    ARMV8_PMUV3_PERF_HW_MEM_UNALIGNED_ACCESS       = 0x0F,
    ARMV8_PMUV3_PERF_HW_PC_BRANCH_MIS_PRED         = 0x10, /* branch forecast miss event */
    ARMV8_PMUV3_PERF_HW_CLOCK_CYCLES               = 0x11, /* cycles event */
    ARMV8_PMUV3_PERF_HW_PC_BRANCH_PRED             = 0x12,
    ARMV8_PMUV3_PERF_HW_MEM_ACCESS                 = 0x13,
    ARMV8_PMUV3_PERF_HW_L1_ICACHE_ACCESS           = 0x14, /* icache access event */
    ARMV8_PMUV3_PERF_HW_L1_DCACHE_WB               = 0x15,
    ARMV8_PMUV3_PERF_HW_L2_CACHE_ACCESS            = 0x16,
    ARMV8_PMUV3_PERF_HW_L2_CACHE_REFILL            = 0x17,
    ARMV8_PMUV3_PERF_HW_L2_CACHE_WB                = 0x18,
    ARMV8_PMUV3_PERF_HW_BUS_ACCESS                 = 0x19,
    ARMV8_PMUV3_PERF_HW_MEM_ERROR                  = 0x1A,
    ARMV8_PMUV3_PERF_HW_OP_SPEC                    = 0x1B,
    ARMV8_PMUV3_PERF_HW_TTBR_WRITE                 = 0x1C,
    ARMV8_PMUV3_PERF_HW_BUS_CYCLES                 = 0x1D,
    ARMV8_PMUV3_PERF_HW_CHAIN                      = 0x1E,
    ARMV8_PMUV3_PERF_HW_L2D_CACHE_ALLOCATE         = 0x20,
    ARMV8_PMUV3_PERF_HW_BR_RETIRED                 = 0x21,
    ARMV8_PMUV3_PERF_HW_BR__MIS_PRED_RETIRED       = 0x22,
    ARMV8_PMUV3_PERF_HW_STALL_FRONTEND             = 0x23,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND              = 0x24,
    ARMV8_PMUV3_PERF_HW_L1D_TLB                    = 0x25,
    ARMV8_PMUV3_PERF_HW_L1I_TLB                    = 0x26,
    ARMV8_PMUV3_PERF_HW_L3D_CACHE_ALLOCATE         = 0x29,
    ARMV8_PMUV3_PERF_HW_L3D_CACHE_REFILL           = 0x2A,
    ARMV8_PMUV3_PERF_HW_L3D_CACHE                  = 0x2B,
    ARMV8_PMUV3_PERF_HW_L2D_TLB_REFILL             = 0x2D,
    ARMV8_PMUV3_PERF_HW_L2D_TLB                    = 0x2F,
    ARMV8_PMUV3_PERF_HW_DTLB_WALK                  = 0x34,
    ARMV8_PMUV3_PERF_HW_ITLB_WALK                  = 0x35,
    ARMV8_PMUV3_PERF_HW_LL_CACHE_RD                = 0x36,
    ARMV8_PMUV3_PERF_HW_LL_CACHE_MISS_RD           = 0x37,
    ARMV8_PMUV3_PERF_HW_REMOTE_ACCESS_RD           = 0x38,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_LD               = 0x40,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_ST               = 0x41,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_REFILL_LD        = 0x42,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_REFILL_ST        = 0x43,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_REFILL_INNER     = 0x44,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_REFILL_OUTER     = 0x45,
    ARMV8_PMUV3_PERF_HW_L2D_CACHE_ST               = 0x51,
    ARMV8_PMUV3_PERF_HW_L2D_CACHE_REFILL_LD        = 0x52,
    ARMV8_PMUV3_PERF_HW_L2D_CACHE_REFILL_ST        = 0x53,
    ARMV8_PMUV3_PERF_HW_BUS_ACCESS_LD              = 0x60,
    ARMV8_PMUV3_PERF_HW_BUS_ACCESS_ST              = 0x61,
    ARMV8_PMUV3_PERF_HW_MEM_ACCESS_LD              = 0x66,
    ARMV8_PMUV3_PERF_HW_MEM_ACCESS_ST              = 0x67,
    ARMV8_PMUV3_PERF_HW_LD_SPEC                    = 0x70,
    ARMV8_PMUV3_PERF_HW_ST_SPEC                    = 0x71,
    ARMV8_PMUV3_PERF_HW_LDST_SPEC                  = 0x72,
    ARMV8_PMUV3_PERF_HW_DP_SPEC                    = 0x73,
    ARMV8_PMUV3_PERF_HW_ASE_SPEC                   = 0x74,
    ARMV8_PMUV3_PERF_HW_VFP_SPEC                   = 0x75,
    ARMV8_PMUV3_PERF_HW_PC_WRITE_SPEC              = 0x76,
    ARMV8_PMUV3_PERF_HW_CRYPTO_SPEC                = 0x77,
    ARMV8_PMUV3_PERF_HW_BR_IMMED_SPEC              = 0x78,
    ARMV8_PMUV3_PERF_HW_BR_RETURN_SPEC             = 0x79,
    ARMV8_PMUV3_PERF_HW_BR_INDIRECT_SPEC           = 0x7A,
    ARMV8_PMUV3_PERF_HW_EXC_IRQ                    = 0x86,
    ARMV8_PMUV3_PERF_HW_EXC_FIQ                    = 0x87,
    ARMV8_PMUV3_PERF_HW_L3D_CACHE_RD               = 0xA0,
    ARMV8_PMUV3_PERF_HW_L3D_CACHE_REFILL_RD        = 0xA2,
    ARMV8_PMUV3_PERF_HW_L3D_CACHE_REFILL_PREFETCH  = 0xC0,
    ARMV8_PMUV3_PERF_HW_L2D_CACHE_REFILL_PREFETCH  = 0xC1,
    ARMV8_PMUV3_PERF_HW_L1D_CACHE_REFILL_PREFETCH  = 0xC2,
    ARMV8_PMUV3_PERF_HW_L2D_WS_MODE                = 0xC3,
    ARMV8_PMUV3_PERF_HW_L1D_WS_MODE_ENTRY          = 0xC4,
    ARMV8_PMUV3_PERF_HW_L1D_WS_MODE                = 0xC5,
    ARMV8_PMUV3_PERF_HW_PREDECODE_ERROR            = 0xC6,
    ARMV8_PMUV3_PERF_HW_L3D_WS_MODE                = 0xC7,
    ARMV8_PMUV3_PERF_HW_BR_COND_PRED               = 0xC9,
    ARMV8_PMUV3_PERF_HW_BR_INDIRECT_MIS_PRED       = 0xCA,
    ARMV8_PMUV3_PERF_HW_BR_INDIRECT_ADDR_MIS_PRED  = 0xCB,
    ARMV8_PMUV3_PERF_HW_BR_COND_MIS_PRED           = 0xCC,
    ARMV8_PMUV3_PERF_HW_BR_INDIRECT_ADDR_PRED      = 0xCD,
    ARMV8_PMUV3_PERF_HW_BR_RETURN_ADDR_PRED        = 0xCE,
    ARMV8_PMUV3_PERF_HW_BR_RETURN_ADDR_MIS_PRED    = 0xCF,
    ARMV8_PMUV3_PERF_HW_L2D_LLWALK_TLB             = 0xD0,
    ARMV8_PMUV3_PERF_HW_L2D_LLWALK_TLB_REFILL      = 0xD1,
    ARMV8_PMUV3_PERF_HW_L2D_L2WALK_TLB             = 0xD2,
    ARMV8_PMUV3_PERF_HW_L2D_L2WALK_TLB_REFILL      = 0xD3,
    ARMV8_PMUV3_PERF_HW_L2D_S2_TLB                 = 0xD4,
    ARMV8_PMUV3_PERF_HW_L2D_S2_TLB_REFILL          = 0xD5,
    ARMV8_PMUV3_PERF_HW_L2D_CACHE_STASH_DROPPED    = 0xD6,
    ARMV8_PMUV3_PERF_HW_STALL_FRONTEND_CACHE       = 0xE1,
    ARMV8_PMUV3_PERF_HW_STALL_FRONTEND_TLB         = 0xE2,
    ARMV8_PMUV3_PERF_HW_STALL_FRONTEND_PDERR       = 0xE3,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_ILOCK        = 0xE4,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_ILOCK_AGU    = 0xE5,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_ILOCK_FPU    = 0xE6,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_LD           = 0xE7,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_ST           = 0xE8,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_LD_CACHE     = 0xE9,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_LD_TLB       = 0xEA,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_ST_STB       = 0xEB,
    ARMV8_PMUV3_PERF_HW_STALL_BACKEND_ST_TLB       = 0xEC,
    ARMV8_PMUV3_PERF_HW_MAX
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* ARMV8_PMU_H */
