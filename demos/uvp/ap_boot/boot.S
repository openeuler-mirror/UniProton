# Start an Application Processor. This must be placed on a 4KB boundary
# somewhere in the 1st MB of conventional memory (APBOOTSTRAP). The AP
# starts in real-mode, with
#    CS selector set to the startup memory address/16;
#    CS base set to startup memory address;
#    CS limit set to 64KB;
#    CPL and IP set to 0.
#

.set PROT_MODE_CS, 0x8
.set PROT_MODE_DS, 0x10
.set CR0_PE,       0x1

.section .text

.code16

.globl ap_boot_16

ap_boot_16:
    /* 关中断 */
    cli
    /* 向高地址增长 */
    cld

    /* 设置段寄存器 */
    xorw %ax, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss

    /* 加载GDT */
    lgdt temp_gdt32_ptr

    /* 开启保护模式 */
    movl %cr0, %eax
    orl  $CR0_PE, %eax
    movl %eax, %cr0

    /* 切换到32位模式 */
    ljmp $PROT_MODE_CS, $ap_boot_32

.code32
ap_boot_32:
    /* 设置保护模式下的段选择器 */
    movw $PROT_MODE_DS, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    /* Enable PAE PGE */
    movl %cr4, %eax
    bts  $5, %eax
    bts  $7, %eax
    mov  %eax, %cr4

    /* load our PML4 */
    mov $0xa0008, %eax
    mov %eax, %cr3

    /* enable long mode */
    mov $0xC0000080, %ecx
    rdmsr
    bts $8, %eax
    wrmsr

    wbinvd

    /* activate long mode by enabling paging */
    mov %cr0, %eax
    bts $31, %eax
    bts $16, %eax
    mov %eax, %cr0

    lgdt temp_gdt64_ptr
    ljmp $PROT_MODE_CS, $ap_boot_64

temp_gdt32:
    .quad 0x0000000000000000
    .quad 0x00cf9f000000ffff
    .quad 0x00cf93000000ffff

temp_gdt32_ptr:
    .word 3 * 8 - 1
    .long temp_gdt32

temp_gdt64:
    .quad 0x0000000000000000
    .quad 0x00af9a000000ffff
    .quad 0x008f92000000ffff

temp_gdt64_ptr:
    .word 3 * 8 - 1
    .long temp_gdt64

.code64

ap_boot_64:
    mov $0x0, %ax
    mov %ax, %fs
    mov %ax, %gs
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %ss
    mov %ax, %es

    wbinvd

    movq $0xf02600000, %rcx
    jmp *%rcx

ap_sleep:
    hlt
    jmp ap_sleep /* die */