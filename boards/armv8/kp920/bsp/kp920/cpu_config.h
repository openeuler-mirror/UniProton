#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H

#include "cache_asm.h"
#include "prt_gic_external.h"

/* 系统内存空间1 */
#if defined(OS_OPTION_RSC_TABLE)
    #define MMU_IMAGE_ADDR             0x51000000ULL
    #define MMU_IMAGE_ADDR_LEN         0x1000000
    #define MMU_OPENAMP_ADDR           0x50000000ULL
    #define OPENAMP_SHM_SIZE           0x1000000
#else
    #define MMU_IMAGE_ADDR             0x202783000000ULL
    #define MMU_IMAGE_ADDR_LEN         0x7D000000           /* 2000MB */
    #define MMU_OPENAMP_ADDR           0x202780000000ULL
    #define OPENAMP_SHM_SIZE           0x30000
#endif

#define MMU_GIC_ADDR               0xAA000000ULL        /* gicd */
#define MMU_GICR0_ADDR             0xAA000000ULL        /* gicr die1 */
#define MMU_GICR1_ADDR             0xAE000000ULL        /* gicr die0 */
#define MMU_GIC_ADDR_LEN           0x1000000
#ifdef OS_GDB_STUB
#define MMU_GDB_STUB_ADDR          0x202780030000ULL
#endif
#define MMU_DMA_ADDR               0x202780100000ULL
#define MMU_DMA_ADDR_LEN           0xA00000
#define MMU_LPI_PEND_ADDR          0x202780B00000ULL /* 预留使用 */
#define MMU_LPI_PEND_ADDR_LEN      0x200000

/* 系统硬件IO空间 */
#define MMU_ECAM_ADDR               0xd0000000ULL
#define MMU_ECAM_ADDR_LEN           0x10000000ULL /* 256Bus * 32Device * 8Fuc * 4KB */
#define MMU_UART_ADDR               0x94080000ULL /* 未验证，未启用 */
#define MMU_UART_ADDR_LEN           0x4000
#define MMU_ITS_ADDR                0xA8100000ULL
#define MMU_ITS1_ADDR               0x2000A8100000ULL
#define MMU_ITS_ADDR_LEN            0x100000ULL /* 1MB */

#define MMU_INVALID_ADDR            0x0ULL

#define UART_BASE_ADDR             MMU_UART_ADDR

#define TEST_CLK_INT               30

#define OS_GIC_VER                 3
#define SICR_ADDR_OFFSET_PER_CORE  0x40000U
#define GIC_REG_BASE_ADDR          MMU_GIC_ADDR

#define GICD_CTLR_S_ADDR           (GIC_REG_BASE_ADDR + 0x0000U)
#define GICD_IGROUPN_ADDR          (GIC_REG_BASE_ADDR + 0x0080U)
#define GICD_ISENABLER0_ADDR       (GIC_REG_BASE_ADDR + 0x0100U)
#define GICD_ICENABLER0_ADDR       (GIC_REG_BASE_ADDR + 0x0180U)
#define GICD_IPRIORITYN_ADDR       (GIC_REG_BASE_ADDR + 0x0400U)
#define GICD_IGRPMODRN_ADDR        (GIC_REG_BASE_ADDR + 0x0D00U)

#define GICR_BASE_SEL ((g_gicCoreMap.value & 0x100000) ? MMU_GICR0_ADDR : MMU_GICR1_ADDR)
#define GICR_BASE0                 (GICR_BASE_SEL + 0x100000U)
#define GICR_BASE1                 (GICR_BASE_SEL + 0x110000U)

#define GICR_CTRL_ADDR             (GICR_BASE0 + 0x0000U)  /* GICR_CTLR_ADDR */
#define GICR_IIDR_ADDR             (GICR_BASE0 + 0x0004U)
#define GICR_TYPER_ADDR            (GICR_BASE0 + 0x0008U)  /* 64bit */
#define GICR_STATUSR_ADDR          (GICR_BASE0 + 0x0010U)
#define GICR_WAKER_ADDR            (GICR_BASE0 + 0x0014U)
#define GICR_PROPBASER_ADDR        (GICR_BASE0 + 0x0070U)  /* 64bit */
#define GICR_PENDBASER_ADDR        (GICR_BASE0 + 0x0078U)  /* 64bit */

#define GICR_IGROUPR0_ADDR         (GICR_BASE1 + 0x0080U)
#define GICR_ISENABLER0_ADDR       (GICR_BASE1 + 0x0100U)
#define GICR_ICENABLER0_ADDR       (GICR_BASE1 + 0x0180U)
#define GICR_ISPENDR0_ADDR         (GICR_BASE1 + 0x0200U)
#define GICR_ICPENDR0_ADDR         (GICR_BASE1 + 0x0280U)
#define GICR_IGRPMODR0_ADDR        (GICR_BASE1 + 0x0D00U)

#define GITS_BASE                  MMU_ITS_ADDR
#define GITS_IIDR                  (GITS_BASE + 0x0004U)
#define GITS_TYPER                 (GITS_BASE + 0x0008U)
#define GITS_PIDR(n)  (GITS_BASE + (((n) > 3) ? 0xffc0U : 0xffe0U) + (4 * (n)))

#define GITS1_BASE                  MMU_ITS1_ADDR
#define GITS1_IIDR                  (GITS1_BASE + 0x0004U)
#define GITS1_TYPER                 (GITS1_BASE + 0x0008U)
#define GITS1_PIDR(n)  (GITS1_BASE + (((n) > 3) ? 0xffc0U : 0xffe0U) + (4 * (n)))

#define MAX_INT_NUM                387
#define MIN_GIC_SPI_NUM            32
#define SICD_IGROUP_INT_NUM        32
#define SICD_REG_SIZE              4

#define GROUP_MAX_BPR              0x7U
#define GROUP0_BP                  0
#define GROUP1_BP                  0

#define PRIO_MASK_LEVEL            0xFFU

#define ICC_SRE_EL1                S3_0_C12_C12_5
#define ICC_BPR0_EL1               S3_0_C12_C8_3
#define ICC_BPR1_EL1               S3_0_C12_C12_3
#define ICC_IGRPEN1_EL1            S3_0_C12_C12_7
#define ICC_PMR_EL1                S3_0_C4_C6_0

#define PARAS_TO_STRING(x...)      #x
#define REG_ALIAS(x...)            PARAS_TO_STRING(x)

#define GIC_REG_READ(addr)         (*(volatile U32 *)((uintptr_t)(addr)))
#define GIC_REG_WRITE(addr, data)  (*(volatile U32 *)((uintptr_t)(addr)) = (U32)(data))

#endif
