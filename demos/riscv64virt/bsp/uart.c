#include "uart.h"
#include "prt_hwi.h"
#include "prt_sem.h"
#define RECV_BUFFER     1024
#define SEND_BUFFER     1024
#define UART_IRQ_PRIO   3

char buffer_rcv[RECV_BUFFER];
SemHandle rcv_sem;
U32 rcv_head = 0;
U32 rcv_tail = 0;



U32 uart_gets(char* dest,U32 size)
{
    if(size <= RECV_BUFFER) {
        return 0xffffffff;
    }
    U32 i =0;
    if(PRT_SemPend(rcv_sem,OS_WAIT_FOREVER) != OS_OK) {
        return 0xffffffff;
    }
    uintptr_t intSave;
    intSave = PRT_HwiLock();
    while(rcv_head != rcv_tail && rcv_head != '\n') {
        dest[i] = buffer_rcv[rcv_head];
        rcv_head++;
        i++; 
    }
    if(rcv_head != rcv_tail) {
        rcv_head ++;
    }
    dest[i]='\0';
    PRT_HwiRestore(intSave);
    return OS_OK;
}

INLINE void uart_putc_sync(int c) 
{
    while((ReadReg(LSR) & LSR_TX_IDLE) == 0) ;
    WriteReg(THR, c);
}

INLINE int uartgetc(void)
{
  if(ReadReg(LSR) & 0x01){
    // input data is ready.
    return ReadReg(RHR);
  } else {
    return -1;
  }
}


void uart_putstr_sync(char *s)
{
    for(;*s!=0;s++) {
        uart_putc_sync(*s);
    }
}

void uart_rcv_handler(HwiArg arg)
{
    int ch ;
    if( (ch = uartgetc()) ==-1 ) {
        return;
    }
    uintptr_t intSave =  PRT_HwiLock();
    if(rcv_tail +1 == rcv_head) {
        if(ch == '\r' || ch =='\n') {
            uart_putc_sync('\n'); 
            buffer_rcv[rcv_tail] = '\n';
            PRT_SemPost(rcv_sem);
            PRT_HwiRestore(intSave);
            return;
        }
        uart_putc_sync(ch);
        PRT_HwiRestore(intSave);
        return;
    }
    
    if(ch == '\r' || ch =='\n') {
        uart_putc_sync('\n'); 
        buffer_rcv[rcv_tail++] = '\n';
        PRT_SemPost(rcv_sem);
        PRT_HwiRestore(intSave);
        return;
    }
    uart_putc_sync(ch);
    buffer_rcv[rcv_tail++] = ch;
    PRT_HwiRestore(intSave);
    return;
}

U32 uart_start(void)
{
  U32 ret;
  ret = PRT_HwiSetAttr(UART0_IRQ,UART_IRQ_PRIO,OS_HWI_MODE_ENGROSS);
  if(ret != OS_OK) {
    return ret;
  }
  
  ret = PRT_HwiCreate(UART0_IRQ,uart_rcv_handler,0);
  if(ret != OS_OK) {
    return ret;
  }
  
  ret = PRT_HwiEnable(UART0_IRQ);
  if(ret != OS_OK) {
    return ret;
  }

  ret = PRT_SemCreate(0,&rcv_sem);
  if(ret != OS_OK) {
    return ret;
  }
  // enable transmit and receive interrupts.
  WriteReg(IER,IER_RX_ENABLE);
  return OS_OK;
}

void uart_init(void)
{
  rcv_head = 0;
  rcv_tail = 0;
  // disable interrupts.
  WriteReg(IER, 0x00);

  // special mode to set baud rate.
  WriteReg(LCR, LCR_BAUD_LATCH);

  // LSB for baud rate of 38.4K.
  WriteReg(0, 0x03);

  // MSB for baud rate of 38.4K.
  WriteReg(1, 0x00);

  // leave set-baud mode,
  // and set word length to 8 bits, no parity.
  WriteReg(LCR, LCR_EIGHT_BITS);

  // reset and enable FIFOs.
  WriteReg(FCR, FCR_FIFO_ENABLE | FCR_FIFO_CLEAR);

 
}