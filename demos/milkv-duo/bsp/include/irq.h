#ifndef __IRQ__
#define __IRQ__

/* RISC-V */
#define CLINT_BASE              0x74000000
#define PLIC_BASE               0x70000000

/* CLINT */
#define CLINT_TIMECMPL0         (CLINT_BASE + 0x4000)
#define CLINT_TIMECMPH0         (CLINT_BASE + 0x4004)

#define CLINT_MTIME(cnt)             asm volatile("csrr %0, time\n" : "=r"(cnt) :: "memory");

/* PLIC */
#define PLIC_PRIORITY0          (PLIC_BASE + 0x0)
#define PLIC_PRIORITY1          (PLIC_BASE + 0x4)
#define PLIC_PRIORITY2          (PLIC_BASE + 0x8)
#define PLIC_PRIORITY3          (PLIC_BASE + 0xc)
#define PLIC_PRIORITY4          (PLIC_BASE + 0x10)

#define PLIC_PENDING1           (PLIC_BASE + 0x1000)
#define PLIC_PENDING2           (PLIC_BASE + 0x1004)
#define PLIC_PENDING3           (PLIC_BASE + 0x1008)
#define PLIC_PENDING4           (PLIC_BASE + 0x100C)

#define PLIC_ENABLE1            (PLIC_BASE + 0x2000)
#define PLIC_ENABLE2            (PLIC_BASE + 0x2004)
#define PLIC_ENABLE3            (PLIC_BASE + 0x2008)
#define PLIC_ENABLE4            (PLIC_BASE + 0x200C)

#define PLIC_THRESHOLD          (PLIC_BASE + 0x200000)
#define PLIC_CLAIM              (PLIC_BASE + 0x200004)

#endif
