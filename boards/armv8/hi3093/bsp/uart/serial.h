#ifndef __SERIAL_H__
#define __SERIAL_H__

#include "uart_regs.h"
#include "prt_typedef.h"

#define SERIAL_SEL_UART_PORT       2

#define SERIAL_TIMEOUT_MS          10
#define SERIAL_NAME_SIZE           32
#define SERIAL_FLUSH_TMOUT         0x100000

typedef struct _serial_cfg {
    char name[SERIAL_NAME_SIZE];
    S32 init_done;
    S32 hw_uart_no;
    U32 uart_src_clk;
    U8 data_bits;
    U8 stop;
    U8 pen;
    U8 eps;
    S32 baud_rate;
} serial_cfg;

typedef struct {
    S32 (*init)(serial_cfg *cfg);
    void (*setbrg)(serial_cfg *cfg);
    S32 (*put_char)(S32 hw_uart_no, const char ch);
    S32 (*get_char)(S32 hw_uart_no);
    S32 (*tx_ready)(S32 hw_uart_no);
    S32 (*rx_ready)(S32 hw_uart_no);
    S32 (*wait4idle)(S32 hw_uart_no, U32 time_out);
} uart_ops;

typedef struct _serial {
    serial_cfg cfg;
    uart_ops *hw_ops;
} serial_t;

extern uart_ops g_uart_ops;
extern serial_cfg g_uart_cfg;

extern void serial_soft_init(serial_cfg *cfg, uart_ops *hw_ops);
extern void serial_init(serial_cfg *cfg, uart_ops *hw_ops);
extern serial_t *serial_get(void);
extern void serial_putc(const char ch);
extern S32 serial_getc(void);
extern void serial_puts(const char *s);
extern S32 serial_tstc(void);
extern void serial_flush(void);
extern void printf_resource_init(void);
extern S32 uart_init(serial_cfg *cfg);
extern void uart_raw_puts(S32 uart_no, const S8 *str);
extern S32 uart_busy_poll(S32 hw_uart_no);

#endif /* __SERIAL_H__ */