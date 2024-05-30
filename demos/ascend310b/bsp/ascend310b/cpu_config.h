#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H

#include "cache_asm.h"

#if defined(GUEST_OS)
/* 虚拟化场景均采用UART2，在main函数初始化相关配置，波特率115200 */
#define UART_BASE_ADDR              0x82230000U
#define UART_CLK_INPUT              150000000U  /* 150M */
#define GPIO_UTXD2_ADDR             0x82320040U
#define GPIO_URXD2_ADDR             0x82320044U
#define TEST_CLK_INT                27
#define OS_GIC_BASE_ADDR            0x7FFF000000ULL     // GICD_BASE_ADDR
#define OS_GICR_OFFSET              0x40000U            // GICR相对于GIC基地址偏移量配置
#define GITS_BASE_ADDR              0x7FFF020000ULL     // GITS基地址
#define UART_INT_NUM                0xcb
#define SHM_WR_ADDR                 0x2AC00000          // 共享内存地址
#else
#define UART_BASE_ADDR              0xC4010000ULL
#define TEST_CLK_INT                30
#define OS_GIC_BASE_ADDR            0xd0000000ULL        // GICD_BASE_ADDR
#define OS_GICR_OFFSET              0x100000U            // GICR相对于GIC基地址偏移量配置
#define MMU_IMAGE_ADDR              0x52000000ULL
#define MMU_GIC_ADDR                OS_GIC_BASE_ADDR
#define MMU_UART_ADDR               UART_BASE_ADDR
#define MMU_OPENAMP_ADDR            0x50000000ULL
#define GITS_BASE_ADDR              0xD2000000ULL       // GITS基地址
#define OPENAMP_SHM_SIZE            0x2000000
#define MMU_LOG_MEM_ADDR            0x53000000ULL
#endif

#define SICR_ADDR_OFFSET_PER_CORE   0x200U
#define OS_GICR_STRIDE              0x40000U            // GICR核间偏移量配置

#define GICD_CTLR_S_ADDR            (OS_GIC_BASE_ADDR + 0x0000U)
#define GICD_IGROUPN_ADDR           (OS_GIC_BASE_ADDR + 0x0080U)
#define GICD_ISENABLER0_ADDR        (OS_GIC_BASE_ADDR + 0x0100U)
#define GICD_ICENABLER0_ADDR        (OS_GIC_BASE_ADDR + 0x0180U)
#define GICD_IPRIORITYN_ADDR        (OS_GIC_BASE_ADDR + 0x0400U)
#define GICD_IGRPMODRN_ADDR         (OS_GIC_BASE_ADDR + 0x0D00U)

#define GICR_BASE0                  (OS_GIC_BASE_ADDR + OS_GICR_OFFSET) // GICR的基地址
#define GICR_BASE1                  (GICR_BASE0 + 0x10000U) // GICR_SGI_OFFSET

#define GICR_CTRL_ADDR              (GICR_BASE0 + 0x0000U)
#define GICR_WAKER_ADDR             (GICR_BASE0 + 0x0014U)

#define GICR_IGROUPR0_ADDR          (GICR_BASE1 + 0x0080U)
#define GICR_ISENABLER0_ADDR        (GICR_BASE1 + 0x0100U)
#define GICR_ICENABLER0_ADDR        (GICR_BASE1 + 0x0180U)
#define GICR_IGRPMODR0_ADDR         (GICR_BASE1 + 0x0D00U)

#define GIC_DIST_BASE               OS_GIC_BASE_ADDR
#define GIC_CPU_BASE                (GIC_DIST_BASE + 0x1000U)

#define GICD_CTLR                   (GIC_DIST_BASE + 0x0000U)
#define GICD_TYPER                  (GIC_DIST_BASE + 0x0004U)
#define GICD_IIDR                   (GIC_DIST_BASE + 0x0008U)
#define GICD_IGROUPRn               (GIC_DIST_BASE + 0x0080U)
#define GICD_ISENABLERn             (GIC_DIST_BASE + 0x0100U)
#define GICD_ICENABLERn             (GIC_DIST_BASE + 0x0180U)
#define GICD_ISPENDRn               (GIC_DIST_BASE + 0x0200U)
#define GICD_ICPENDRn               (GIC_DIST_BASE + 0x0280U)
#define GICD_ISACTIVERn             (GIC_DIST_BASE + 0x0300U)
#define GICD_ICACTIVERn             (GIC_DIST_BASE + 0x0380U)
#define GICD_IPRIORITYn             (GIC_DIST_BASE + 0x0400U)

#define GICC_CTLR                   (GIC_CPU_BASE + 0x0000U)
#define GICC_PMR                    (GIC_CPU_BASE + 0x0004U)

#define BIT(n)                      (1 << (n))

#define GICC_CTLR_ENABLEGRP0        BIT(0)
#define GICC_CTLR_ENABLEGRP1        BIT(1)
#define GICC_CTLR_FIQBYPDISGRP0     BIT(5)
#define GICC_CTLR_IRQBYPDISGRP0     BIT(6)
#define GICC_CTLR_FIQBYPDISGRP1     BIT(7)
#define GICC_CTLR_IRQBYPDISGRP1     BIT(8)

#define GICC_CTLR_ENABLE_MASK       (GICC_CTLR_ENABLEGRP0 | \
                                     GICC_CTLR_ENABLEGRP1)

#define GICC_CTLR_BYPASS_MASK       (GICC_CTLR_FIQBYPDISGRP0 | \
                                     GICC_CTLR_IRQBYPDISGRP0 | \
                                     GICC_CTLR_FIQBYPDISGRP1 | \
                                     GICC_CTLR_IRQBYPDISGRP1)

#define GIC_REG_READ(addr)          (*(volatile U32 *)((uintptr_t)(addr)))
#define GIC_REG_WRITE(addr, data)   (*(volatile U32 *)((uintptr_t)(addr)) = (U32)(data))

#define MAX_INT_NUM                 387
#define MIN_GIC_SPI_NUM             32
#define SICD_IGROUP_INT_NUM         32
#define SICD_REG_SIZE               4

#define GROUP_MAX_BPR               0x7U
#define GROUP0_BP                   0
#define GROUP1_BP                   0

#define PRIO_MASK_LEVEL             0xFFU

#define ICC_SRE_EL1                S3_0_C12_C12_5
#define ICC_BPR0_EL1               S3_0_C12_C8_3
#define ICC_BPR1_EL1               S3_0_C12_C12_3
#define ICC_IGRPEN1_EL1            S3_0_C12_C12_7
#define ICC_PMR_EL1                S3_0_C4_C6_0

#define PARAS_TO_STRING(x...)       #x
#define REG_ALIAS(x...)             PARAS_TO_STRING(x)

#endif
