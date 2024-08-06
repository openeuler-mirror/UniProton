#ifndef CPU_CONFIG_H
#define CPU_CONFIG_H

#include "cache_asm.h"

#define OPENAMP_SHARED_MEM_LENGTH   0x2000000U
#define OPENAMP_LOG_LENGTH    0x200000U

#define MMU_IMAGE_ADDR        0x7A000000ULL
#define MMU_OPENAMP_ADDR      (MMU_IMAGE_ADDR - OPENAMP_SHARED_MEM_LENGTH - OPENAMP_LOG_LENGTH)

#ifdef OS_GDB_STUB
#define GDB_STUB_BUFFER_SHIFT  0x4000
#define MMU_GDB_STUB_ADDR      (MMU_IMAGE_ADDR - GDB_STUB_BUFFER_SHIFT)
#endif

#define MMU_GIC_ADDR          0xFE600000ULL

#define MMU_PMU1_IOC_ADDR     0xFD5F0000ULL
#define MMU_PMU2_IOC_ADDR     0xFD5F4000ULL
#define MMU_BUS_IOC_ADDR      0xFD5F8000ULL

#define MMU_CRU_ADDR          0xFD7C0000ULL
#define MMU_PMU1_CRU_ADDR     0xFD7F0000ULL

#define MMU_GPIO0_ADDR        0xFD8A0000ULL
#define MMU_GPIO1_ADDR        0xFEC20000ULL
#define MMU_GPIO2_ADDR        0xFEC30000ULL
#define MMU_GPIO3_ADDR        0xFEC40000ULL
#define MMU_GPIO4_ADDR        0xFEC50000ULL

#define MMU_UART0_ADDR        0xFD890000ULL
// #define MMU_UART1_ADDR        0xFEB40000ULL
#define MMU_UART2_ADDR        0xFEB50000ULL
// #define MMU_UART3_ADDR        0xFEB60000ULL
// #define MMU_UART4_ADDR        0xFEB70000ULL
// #define MMU_UART5_ADDR        0xFEB80000ULL
// #define MMU_UART6_ADDR        0xFEB90000ULL
// #define MMU_UART7_ADDR        0xFEBA0000ULL
// #define MMU_UART8_ADDR        0xFEBB0000ULL
// #define MMU_UART9_ADDR        0xFEBC0000ULL

// #define MMU_CAN0_ADDR         0xFEA50000ULL
#define MMU_CAN1_ADDR         0xFEA60000ULL
// #define MMU_CAN2_ADDR         0xFEA70000ULL

#define MMU_I2C0_ADDR         0xFD880000ULL
// #define MMU_I2C1_ADDR         0xFEA90000ULL
// #define MMU_I2C2_ADDR         0xFEAA0000ULL
// #define MMU_I2C3_ADDR         0xFEAB0000ULL
// #define MMU_I2C4_ADDR         0xFEAC0000ULL
// #define MMU_I2C5_ADDR         0xFEAD0000ULL
// #define MMU_I2C6_ADDR         0xFEC80000ULL
// #define MMU_I2C7_ADDR         0xFEC90000ULL
// #define MMU_I2C8_ADDR         0xFECA0000ULL

#define MMU_TIMER_ADDR        0xFEAE0000ULL

#define MMU_SPI0_ADDR         0xFEB00000ULL
// #define MMU_SPI1_ADDR         0xFEB10000ULL
// #define MMU_SPI2_ADDR         0xFEB20000ULL
// #define MMU_SPI3_ADDR         0xFEB30000ULL
// #define MMU_SPI4_ADDR         0xFECB0000ULL

#define TEST_CLK_INT               30

#define OS_GIC_VER                 3
#define SICR_ADDR_OFFSET_PER_CORE  0x20000U
#define GIC_REG_BASE_ADDR          0xFE600000ULL

#define GICD_CTLR_S_ADDR           (GIC_REG_BASE_ADDR + 0x0000U)
#define GICD_IGROUPN_ADDR          (GIC_REG_BASE_ADDR + 0x0080U)
#define GICD_ISENABLER0_ADDR       (GIC_REG_BASE_ADDR + 0x0100U)
#define GICD_ICENABLER0_ADDR       (GIC_REG_BASE_ADDR + 0x0180U)
#define GICD_IPRIORITYN_ADDR       (GIC_REG_BASE_ADDR + 0x0400U)
#define GICD_IGRPMODRN_ADDR        (GIC_REG_BASE_ADDR + 0x0D00U)
#define GICD_IROUTER               (GIC_REG_BASE_ADDR + 0x6100U)

#define GICR_BASE0                 (GIC_REG_BASE_ADDR + 0x80000U)
#define GICR_BASE1                 (GIC_REG_BASE_ADDR + 0x90000U)

#define GICR_CTRL_ADDR             (GICR_BASE0 + 0x0000U)
#define GICR_WAKER_ADDR            (GICR_BASE0 + 0x0014U)

#define GICR_IGROUPR0_ADDR         (GICR_BASE1 + 0x0080U)
#define GICR_ISENABLER0_ADDR       (GICR_BASE1 + 0x0100U)
#define GICR_ICENABLER0_ADDR       (GICR_BASE1 + 0x0180U)
#define GICR_IGRPMODR0_ADDR        (GICR_BASE1 + 0x0D00U)

#define MAX_INT_NUM                450
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
