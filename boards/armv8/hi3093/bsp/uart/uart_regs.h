#ifndef __DW_UART_REGS_H__
#define __DW_UART_REGS_H__

#define UART0_REG_BASE 0x0872A000
#define UART1_REG_BASE 0x0872B000
#define UART2_REG_BASE 0x08710000
#define UART3_REG_BASE 0x08711000
#define UART4_REG_BASE 0x08743000
#define UART5_REG_BASE 0x08744000
#define UART6_REG_BASE 0x0875D000
#define UART7_REG_BASE 0x0875E000

#define CCORE_SYS_UART2_INTID 90
#define CCORE_SYS_UART3_INTID 91
#define CCORE_SYS_UART4_INTID 92
#define CCORE_SYS_UART5_INTID 93
#define CCORE_SYS_UART6_INTID 94
#define CCORE_SYS_UART7_INTID 95

/*
 * UART register offsets
 */
#define DW_UART_RBR 0x00
#define DW_UART_THR 0x00
#define DW_UART_DLL 0x00

#define DW_UART_DLH 0x04
#define DW_UART_IER 0x04

#define DW_UART_IIR 0x08
#define DW_UART_FCR 0x08

#define DW_UART_LCR 0x0C

#define DW_UART_MCR 0x10

#define DW_UART_LSR 0x14
#define DW_UART_MSR 0x18

#define DW_UART_USR 0x7C
#define DW_UART_TFL 0x80
#define DW_UART_RFL 0x84

#define DW_UART_HTX 0xA4

/* LSR 线性状态寄存器区域 */
#define DW_UART_LSR_TEMT 0x40
#define DW_UART_LSR_THRE 0x20
#define DW_UART_LSR_BI   0x10
#define DW_UART_LSR_FE   0x08
#define DW_UART_LSR_PE   0x04
#define DW_UART_LSR_R    0x02
#define DW_UART_LSR_DDR  0x01

#define BIT(n)           (1 << (n))

/* LCR bit field */
#define DW_UART_DLAB         BIT(7)
#define DW_UART_BREAK        BIT(6)
#define DW_UART_STICK        BIT(5)
#define DW_UART_EPS          BIT(4)
#define DW_UART_PEN          BIT(3)
#define DW_UART_STOP         BIT(2)
#define DW_UART_8bit         0x3
#define DW_UART_7bit         0x2
#define DW_UART_6bit         0x1
#define DW_UART_5bit         0x0   
#define DW_UART_DATALEN_MASK 0x03

/* IER bit field */
#define PTIME  BIT(7)
#define EDSSI  BIT(3)
#define ELSI   BIT(2)
#define ETBEI  BIT(1)
#define ERBFI  BIT(0)

/* LSR bit field */
#define DW_RFE   BIT(7)
#define DW_TEMT  BIT(6)
#define DW_THRE  BIT(5)
#define DW_BI    BIT(4)
#define DW_FE    BIT(3)
#define DW_PE    BIT(2)
#define DW_OE    BIT(1)
#define DW_DR    BIT(0)

#define UART_DW_DR_PE DW_PE
#define UART_DW_DR_FE DW_FE
#define UART_DW_DR_OE DW_OE
#define UART_DW_DR_BE DW_BI

#define DW_RSR_ANY  (DW_OE | DW_PE | DW_FE | DW_BI)
#define DW_DUMMY_RSR_RX

/* MCR bit field */
#define DW_MC_AFCE  BIT(5)
#define DW_MC_LOOP  BIT(4)
#define DW_MC_OUT2  BIT(3)
#define DW_MC_OUT1  BIT(2)
#define DW_MC_RTS   BIT(1)
#define DW_MC_DTR   BIT(0)

/* MSR bit field */
#define DW_DCD     BIT(7)
#define DW_RI      BIT(6)
#define DW_DSR     BIT(5)
#define DW_CTS     BIT(4)
#define DW_MSR_ANY (DW_DCD | DW_DSR | DW_CTS)

/* IIR bit field */
#define DW_RECEIVERR       0x06
#define DW_RECEIVEAVA      0x04
#define DW_RECTIMEOUT      0x0C
#define DW_TRANSEMP        0x02
#define DW_NOINTERRUPT     0x01
#define DW_MODEMSTA        0x0
#define DW_BUSY            0x7

/* FCR bit field */
#define RECFIFO1_2     (0x02 << 6)
#define TXFIFO1_2      (0x03 << 4) 
#define FIFOENA        1
#define UART_FCR_RXCLR 0x02
#define UART_FCR_TXCLR 0x04

/* USR bit field */
#define DW_UART_BUSY      0x01
#define DW_XFIFO_NOT_FULL 0x02
#define DW_XFIFO_EMP      0x04
#define DW_RFIFO_NOT_EMP  0x08
#define DW_RFIFO_FULL     0x10

#define UART_REG_READ(addr)          (*(volatile U32 *)(((uintptr_t)addr)))
#define UART_REG_WRITE(value, addr)  (*(volatile U32 *)((uintptr_t)addr) = (U32)value)

#endif /* __DW_UART_REGS_H__ */