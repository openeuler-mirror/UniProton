#include "prt_config_internal.h"
#include "prt_config.h"
#include "prt_typedef.h"
#include "os_cpu_riscv64g.h"
#include "riscv.h"
#include "platform.h"
#include "uart.h"
#include "console.h"
extern S32 OsConfigStart(void);
extern void trap();

TskHandle pid;
void OsHwInit()
{
    w_mstatus(r_mstatus() & (~MIE));
    w_mie(r_mie() | MEIE | MSIE | MTIE);
    U64 x = (U64)trap;
    OS_EMBED_ASM("csrw mtvec, %0"::"r"(x):);
    //w_mtvec((U64)trap);
    U64 coreId = PRT_GetCoreID();
    U64 nowTime = *(U64 *)CLINT_TIME;
    *(U64* )CLINT_TIMECMP(coreId) = (nowTime + OS_TICK_PER_SECOND);    
}

void HwTimerIsr(void)
{ 
    U64 coreId = PRT_GetCoreID();
    U64 nowTime = *(U64 *)CLINT_TIME;
    *(U64*)CLINT_TIMECMP(coreId) = nowTime + OS_TICK_PER_SECOND;
    PRT_TickISR();
}

U32 PRT_HardDrvInit(void)
{
    return OS_OK;
}



void thread_read_console(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    static char  buf[1030];
    uart_putstr_sync(">> ");
    while(1) {
        if(uart_gets(buf,1030) == OS_OK) {
            console_handle(buf);
        }
    }
}


U32 PRT_AppInit(void)
{
    uart_init();
    if(uart_start() != OS_OK) {
        uart_putstr_sync("err in uart_start");
        while(1) {
            OS_EMBED_ASM("wfi");
        }
    }
    struct TskInitParam para;
    para.taskEntry = thread_read_console;
    para.taskPrio = 25;
    para.stackSize = 0x1000;
    para.name ="console thread";
    para.stackAddr = 0;
    para.args[0] = 0;
    para.args[1] = 0;
    para.args[2] = 0;
    para.args[3] = 0;
    
    if(PRT_TaskCreate(&pid,&para) != OS_OK) {
        uart_putstr_sync("err in prt_task_create");
        while(1) {
            OS_EMBED_ASM("wfi");
        }
    }
    if(PRT_TaskResume(pid) != OS_OK ) {
        uart_putstr_sync("err in prt_task_resume");
        while(1) {
            OS_EMBED_ASM("wfi");
        }
    }
    return OS_OK;
}
int main()
{
    return OsConfigStart();
}

extern void *__wrap_memset(void *dest, int set, U32 len)
{
    if (dest == NULL || len == 0) {
        return NULL;
    }
    
    char *ret = (char *)dest;
    for (int i = 0; i < len; ++i) {
        ret[i] = set;
    }
    return dest;
}

extern void *__wrap_memcpy(void *dest, const void *src, size_t size)
{
    for (size_t i = 0; i < size; ++i) {
        *(char *)(dest + i) = *(char *)(src + i);
    }
    return dest;
}
