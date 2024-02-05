#ifndef __PLATFORM_H
#define __PLATFORM_H


#define UART0 0x10000000L
#define UART0_IRQ 10

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

#define PLIC    0x0c000000L
#define CLINT   0x2000000L

#define CLINT_MSI              (CLINT)
#define CLINT_TIME             (CLINT+0xBFF8)
#define CLINT_TIMECMP(hart_id) (CLINT+0x4000+8*(hart_id))

#endif 