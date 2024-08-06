#include "cpu_config.h"
#include "mmu.h"

mmu_mmap_region_s g_mem_map_info[] = {
    {
        .virt = MMU_IMAGE_ADDR,
        .phys = MMU_IMAGE_ADDR,
        .size = 0x800000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_CACHE_SHARE | MMU_ACCESS_RWX,
    },

    {
        .virt = MMU_GIC_ADDR,
        .phys = MMU_GIC_ADDR,
        .size = 0x1000000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },

#if defined(MMU_OPENAMP_ADDR)
    {
        .virt = MMU_OPENAMP_ADDR,
        .phys = MMU_OPENAMP_ADDR,
        .size = 0x30000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GDB_STUB_ADDR)
    {
        .virt = MMU_GDB_STUB_ADDR,
        .phys = MMU_GDB_STUB_ADDR,
        .size = 0x4000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PMU1_IOC_ADDR)
    {
        .virt = MMU_PMU1_IOC_ADDR,
        .phys = MMU_PMU1_IOC_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PMU2_IOC_ADDR)
    {
        .virt = MMU_PMU2_IOC_ADDR,
        .phys = MMU_PMU2_IOC_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_BUS_IOC_ADDR)
    {
        .virt = MMU_BUS_IOC_ADDR,
        .phys = MMU_BUS_IOC_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CRU_ADDR)
    {
        .virt = MMU_CRU_ADDR,
        .phys = MMU_CRU_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_PMU1_CRU_ADDR)
    {
        .virt = MMU_PMU1_CRU_ADDR,
        .phys = MMU_PMU1_CRU_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART0_ADDR)
    {
        .virt = MMU_UART0_ADDR,
        .phys = MMU_UART0_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART1_ADDR)
    {
        .virt = MMU_UART1_ADDR,
        .phys = MMU_UART1_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART2_ADDR)
    {
        .virt = MMU_UART2_ADDR,
        .phys = MMU_UART2_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART3_ADDR)
    {
        .virt = MMU_UART3_ADDR,
        .phys = MMU_UART3_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART4_ADDR)
    {
        .virt = MMU_UART4_ADDR,
        .phys = MMU_UART4_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART5_ADDR)
    {
        .virt = MMU_UART5_ADDR,
        .phys = MMU_UART5_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART6_ADDR)
    {
        .virt = MMU_UART6_ADDR,
        .phys = MMU_UART6_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART7_ADDR)
    {
        .virt = MMU_UART7_ADDR,
        .phys = MMU_UART7_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART8_ADDR)
    {
        .virt = MMU_UART8_ADDR,
        .phys = MMU_UART8_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_UART9_ADDR)
    {
        .virt = MMU_UART9_ADDR,
        .phys = MMU_UART9_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO0_ADDR)
    {
        .virt = MMU_GPIO0_ADDR,
        .phys = MMU_GPIO0_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO1_ADDR)
    {
        .virt = MMU_GPIO1_ADDR,
        .phys = MMU_GPIO1_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO2_ADDR)
    {
        .virt = MMU_GPIO2_ADDR,
        .phys = MMU_GPIO2_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO3_ADDR)
    {
        .virt = MMU_GPIO3_ADDR,
        .phys = MMU_GPIO3_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_GPIO4_ADDR)
    {
        .virt = MMU_GPIO4_ADDR,
        .phys = MMU_GPIO4_ADDR,
        .size = 0x200,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN0_ADDR)
    {
        .virt = MMU_CAN0_ADDR,
        .phys = MMU_CAN0_ADDR,
        .size = 0x600,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN1_ADDR)
    {
        .virt = MMU_CAN1_ADDR,
        .phys = MMU_CAN1_ADDR,
        .size = 0x600,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_CAN2_ADDR)
    {
        .virt = MMU_CAN2_ADDR,
        .phys = MMU_CAN2_ADDR,
        .size = 0x600,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_TIMER_ADDR)
    {
        .virt = MMU_TIMER_ADDR,
        .phys = MMU_TIMER_ADDR,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
    {
        .virt = MMU_TIMER_ADDR + 0x8000,
        .phys = MMU_TIMER_ADDR + 0x8000,
        .size = 0x100,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI0_ADDR)
    {
        .virt = MMU_SPI0_ADDR,
        .phys = MMU_SPI0_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI1_ADDR)
    {
        .virt = MMU_SPI1_ADDR,
        .phys = MMU_SPI1_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI2_ADDR)
    {
        .virt = MMU_SPI2_ADDR,
        .phys = MMU_SPI2_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI3_ADDR)
    {
        .virt = MMU_SPI3_ADDR,
        .phys = MMU_SPI3_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_SPI4_ADDR)
    {
        .virt = MMU_SPI4_ADDR,
        .phys = MMU_SPI4_ADDR,
        .size = 0x1000,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C0_ADDR)
    {
        .virt = MMU_I2C0_ADDR,
        .phys = MMU_I2C0_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C1_ADDR)
    {
        .virt = MMU_I2C1_ADDR,
        .phys = MMU_I2C1_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C2_ADDR)
    {
        .virt = MMU_I2C2_ADDR,
        .phys = MMU_I2C2_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C3_ADDR)
    {
        .virt = MMU_I2C3_ADDR,
        .phys = MMU_I2C3_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C4_ADDR)
    {
        .virt = MMU_I2C4_ADDR,
        .phys = MMU_I2C4_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C5_ADDR)
    {
        .virt = MMU_I2C5_ADDR,
        .phys = MMU_I2C5_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C6_ADDR)
    {
        .virt = MMU_I2C6_ADDR,
        .phys = MMU_I2C6_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C7_ADDR)
    {
        .virt = MMU_I2C7_ADDR,
        .phys = MMU_I2C7_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

#if defined(MMU_I2C8_ADDR)
    {
        .virt = MMU_I2C8_ADDR,
        .phys = MMU_I2C8_ADDR,
        .size = 0x400,
        .max_level = 0x2,
        .attrs = MMU_ATTR_DEVICE_NGNRNE | MMU_ACCESS_RWX,
    },
#endif

};
