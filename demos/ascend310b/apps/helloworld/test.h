#include "prt_config.h"
#include "prt_task.h"
#include "prt_hwi.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_mem.h"

extern U32 PRT_Printf(const char *format, ...);
extern void uart_poll_send(unsigned char ch);
extern void PRT_UartInit(void);
#define printf PRT_Printf