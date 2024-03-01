/*
 * Copyright (c) 2024-2024 Huawei Technologies Co., Ltd. All rights reserved.
 *
 * UniProton is licensed under Mulan PSL v2.
 * You can use this software according to the terms and conditions of the Mulan PSL v2.
 * You may obtain a copy of Mulan PSL v2 at:
 *          http://license.coscl.org.cn/MulanPSL2
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
 * EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
 * MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
 * See the Mulan PSL v2 for more details.
 * Create: 2024-02-22
 * Description: console控制台相关的代码
 */
#include "uart.h"
#include "prt_typedef.h"
#include <string.h>
#include "prt_sys.h"
#include "platform.h"
#include "prt_hwi.h"
#include "prt_clk.h"
#include "prt_sem.h"
#include "prt_hook.h"
#include "prt_exc.h"
#include "prt_buildef.h"
#include "prt_config.h"
#include "console.h"
#define RCV_BUF_SIZE  1024
static char rcv_buf[RCV_BUF_SIZE];
static U64  tail;
static U64  head;
static U8   need_process_flag;
SemHandle   buf_sem;
static void ch_in_buf(int ch) 
{
    uintptr_t intSave = PRT_HwiLock();
    if(need_process_flag == 1) {
        PRT_HwiRestore(intSave);
        return;
    }
    // buf full
    if(tail +1 == head) {
        need_process_flag=1;
        PRT_SemPost(buf_sem);
        uart_putc('\n');
        PRT_HwiRestore(intSave);
        return;
    }
    PRT_HwiRestore(intSave);
    if(ch == '\b' || ch =='\177')  {
        uart_putc('\b');
        uart_putc(' ');
        uart_putc('\b');
        if(head!=tail) {
            tail--;
        }
        return;
    }
    if(ch == '\n' || ch == '\r') {
        rcv_buf[tail++] = '\n';
        uart_putc('\n');
        need_process_flag=1;
        PRT_SemPost(buf_sem);
        return;
    }
    rcv_buf[tail++] = ch;
    uart_putc(ch);
    return;    
}

void uart_rcv_handler(HwiArg hwi_id)
{
   while(1) {
    int ch = uartgetc();
    if(ch == -1) {
        break;
    }
    ch_in_buf(ch);
   }
}

int console_gets(char* buf, size_t max_size)
{
    (void)max_size;
    PRT_SemPend(buf_sem, OS_WAIT_FOREVER);
    if(head == tail) return 0;
    int i =0;
    for(;head!=tail;head++,i++)
    {
        buf[i] = rcv_buf[head];
    }
    buf[i++]=0;
    uintptr_t intSave = PRT_HwiLock();
    need_process_flag = 0;
    PRT_HwiRestore(intSave);
    return i;
}

void console_init(void)
{
    head = 0;
    tail = 0;
    need_process_flag = 0;
    int ret = PRT_SemCreate(0,&buf_sem);
    if(ret != OS_OK) {
        uart_printf("sem init error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    ret = PRT_HwiSetAttr(UART0_IRQ, UART_PRIO, OS_HWI_MODE_ENGROSS);
    if(ret != OS_OK) {
        uart_printf("hwiSetAttr error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    ret = PRT_HwiCreate(UART0_IRQ, uart_rcv_handler, 0);
    if(ret != OS_OK) {
        uart_printf("hwiSetCreate error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

    ret = PRT_HwiEnable(UART0_IRQ);
    if(ret != OS_OK) {
        uart_printf("hwiEnable error\n");
        while(1) {
            __asm__ __volatile__("wfi");
        }
    }

}


#define EXC_IN_HWI      0
#define EXC_IN_TICK     1
#define EXC_IN_TASK     3
#define EXC_IN_SYS_BOOT 4
#define EXC_IN_SYS      5

enum C_TYPE {
    CONSOLE_WRITE = 0x1452u ,
    CONSOLE_READ            ,
    CONSOLE_BASE_INFO       ,
    CONSOLE_HELP            ,
    CONSOLE_ERROR           ,
    CONSOLE_HWI_TEST        ,
    CONSOLE_HWI_TEST_2      ,
    CONSOLE_HWI_TEST_3      ,
    CONSOLE_EXC_TEST_1      ,
    CONSOLE_EXC_TEST_2      ,
    CONSOLE_HELLO_WORLD     ,
    CONSOLE_HELLO_WORLD_RS  ,
    CONSOLE_HELLO_WORLD_SUSPEND,
    CONSOLE_DEL_HELLO_WORLD,
    CONSOLE_DELAY_HELLO_WORLD,
    CONSOLE_LOCK_TEST,
} ;

#define BUFSIZE 128
static char ibuf[BUFSIZE];

void itoa_self(U64 x, char* p)
{
    int i = 0;
    while (x)
    {
        p[i] = x % 16;
        //二、10进制转16进制
        if (p[i] > 9)//如果得出的16进制的数>9就要考虑转化为字母的情况
        {//16进制范围：0~9,A(16进制代表10，它对应ASC码为65，所以得出的16进制数+55，就可以转换为对应的字母) B(11) C(12) D(13) E(14) F(15)
         //我们算出来的是数字，而我们要存入字符数组（有可能是16进制有字母的原因），所以要从数字转换为字符
            p[i] += 55;//大于9就转换为对应的大写字母
            i++;
        }
        else
        {
            p[i] += 48;//<=9就应把对应的数字转换为字符，因为字符0~9的ASC码对应为48~57,所以数字转换为字符就+48即可
            i++;
        }
        x /= 16;
    }
    p[i] = '\0';
    //三、逆置字符数组，因为10进制转16进制，对应余数是倒着数的
    int left = (p[0] == '-') ? 1 : 0, right = i - 1;//负数的话就要从下一个下标为left倒置
    while (left < right)
    {
        char temp = p[left];
        p[left] = p[right];
        p[right] = temp;
        left++;
        right--;
    }
}

void dump_task(struct TskInfo* task)
{
    uart_putstr_sync("==========task_bef_exc===================\n");
    uart_putstr_sync("task_sp : 0x");
    itoa_self(task->sp,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("task_pc : 0x");
    itoa_self(task->pc,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("task_status : 0x");
    itoa_self(task->taskStatus,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");
    uart_putstr_sync("(here i don't handle this because there is too many flags)\n");
    uart_putstr_sync("(if you want to check, do it yourself)\n");
    
    uart_putstr_sync("taskPrio : 0x");
    itoa_self(task->taskPrio,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");
    
    uart_putstr_sync("stackSize : 0x");
    itoa_self(task->stackSize,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("topOfStack : 0x");
    itoa_self(task->topOfStack,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("taskName : ");
    uart_putstr_sync(task->name);
    uart_putstr_sync("\n");

    uart_putstr_sync("taskEntryAddr : 0x");
    itoa_self((U64)(task->entry),ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("tasktcbAddr : 0x");
    itoa_self((U64)task->tcbAddr,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("taskStackBottom : 0x");
    itoa_self(task->bottom,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("stackCurrUsed : 0x");
    itoa_self(task->currUsed,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("stackPeakUsed : 0x");
    itoa_self(task->peakUsed,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("isOvf : 0x");
    itoa_self(task->ovf,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("=========================================\n");
}

void dump_callee(struct ExcCalleeInfo* callee)
{
    uart_putstr_sync("sp : 0x");
    itoa_self(callee->sp,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s0 : 0x");
    itoa_self(callee->s0,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("fp : 0x");
    itoa_self(callee->fp,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s2 : 0x");
    itoa_self(callee->s2,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s3 : 0x");
    itoa_self(callee->s3,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s4 : 0x");
    itoa_self(callee->s4,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s5 : 0x");
    itoa_self(callee->s5,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");
    
    uart_putstr_sync("s6 : 0x");
    itoa_self(callee->s6,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s7 : 0x");
    itoa_self(callee->s7,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s8 : 0x");
    itoa_self(callee->s8,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s9 : 0x");
    itoa_self(callee->s9,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s10 : 0x");
    itoa_self(callee->s10,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("s11 : 0x");
    itoa_self(callee->s11,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");
}

void dump_cause(struct ExcCauseRegInfo* cause)
{
    uart_putstr_sync("mcause : 0x");
    itoa_self(cause->mcause,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("mepc  : 0x");
    itoa_self(cause->mepc,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("mstatus  : 0x");
    itoa_self(cause->mstatus,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("mtval  : 0x");
    itoa_self(cause->mtval,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");
}

U32 ExcProcFuncTest(struct ExcInfo *excInfo)
{
    uart_putstr_sync("=============================exc_dump_start======================\n");
    uart_putstr_sync("OsVersion : ");
    uart_putstr_sync(excInfo->osVer);
    uart_putstr_sync("\n");

    uart_putstr_sync("AppVersion : ");
    if(excInfo->appVer[0]=='\0')
    {
        uart_putstr_sync("none");
    }
    else 
    {
        uart_putstr_sync(excInfo->osVer);
    }
    uart_putstr_sync("\n");

    uart_putstr_sync("Excption IN :");
    switch(excInfo->threadType)
    {
        case EXC_IN_HWI:
            uart_putstr_sync("hardware interrupt \n");
            break;
        case EXC_IN_TICK:
            uart_putstr_sync("hardware tick interrupt \n");
            break;
        case EXC_IN_SYS:
            uart_putstr_sync("system handler\n");
            break;
        case EXC_IN_TASK:
            uart_putstr_sync("task\n");
            uart_putstr_sync("threadId: ");
            itoa_self(excInfo->threadId,ibuf);
            if(ibuf[0]==0) 
            {
                ibuf[0]='0';
                ibuf[1]=0;
            }
            uart_putstr_sync(ibuf);
            uart_putstr_sync("\n");
            break;
        default:
            uart_putstr_sync("type i don't know\n");
            break;
    }
    uart_putstr_sync("ByteOrder :");
    if(excInfo->byteOrder == OS_LITTLE_ENDIAN)
    {
        uart_putstr_sync("little endian\n");
    }
    else 
    {
        uart_putstr_sync("big endian\n");
    }

    uart_putstr_sync("CpuType : ");
    switch(excInfo->cpuType)
    {
        case OS_RV64_VIRT:
        uart_putstr_sync("riscv64 qemu virt\n");
        break;
        default:
        uart_putstr_sync("unkown type\n");    
        break;
    }

    uart_putstr_sync("coreId : ");
    if(excInfo->coreId ==0)
    {
        uart_putstr_sync("0\n");
    }
    else 
    {
        uart_putstr_sync("unkown\n");
    }

    uart_putstr_sync("cpuTick : 0x");
    itoa_self(excInfo->cpuTick,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("nestCnt : 0x");
    itoa_self(excInfo->nestCnt,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("stackBottom : 0x");
    itoa_self(excInfo->stackBottom,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    dump_cause(&(excInfo->excCause));

    dump_callee(&(excInfo->excContext));

    if(excInfo->task != NULL)
    {
        dump_task(excInfo->task);
    }
    


    uart_putstr_sync("=============================exc_dump_end=======================\n");
    return 0;
}
void HwiEntryHook_test_add_head(U32 hwiNum)
{
    uart_putstr_sync("hook entry !\n");
}
void HwiEntryHook_test_add_tail(U32 hwiNum)
{
    uart_putstr_sync("hook tail !\n");
}

void console_hwi_test_2()
{
    if(PRT_HwiAddEntryHook(HwiEntryHook_test_add_head) != OS_OK)
    {
        uart_putstr_sync("add hook entry error\n");
    }
    else 
    {
        uart_putstr_sync("add hook entry success\n");
    }
    if(PRT_HwiAddExitHook(HwiEntryHook_test_add_tail) != OS_OK)
    {
        uart_putstr_sync("add hook tail error\n");
    }
    else
    {
        uart_putstr_sync("add hook tail success\n");
    }
    uart_putstr_sync(">> ");
}
void console_hwi_test_3()
{
    if(PRT_HwiDelEntryHook(HwiEntryHook_test_add_head) != OS_OK)
    {
        uart_putstr_sync("del hook entry error\n");
    }
    else
    {
        uart_putstr_sync("del hook entry success\n");
    }
    if(PRT_HwiDelExitHook(HwiEntryHook_test_add_tail) != OS_OK)
    {
        uart_putstr_sync("del hook tail error\n");
    }
    else 
    {
        uart_putstr_sync("del hook tail success\n");
    }
    uart_putstr_sync(">> ");
}
void console_exc_test()
{
    if(PRT_ExcRegHook(ExcProcFuncTest)!=OS_OK)
    {
        uart_putstr_sync("add exc hook error\n");
        return;
    }
    uart_putstr_sync("add exc hook success\n");
    uart_putstr_sync(">> ");
}
void console_exc_trig()
{
    OS_EMBED_ASM("ecall");
}
void console_base_info()
{
    char * OsVersion = PRT_SysGetOsVersion();
    uart_putstr_sync("system version : ");
    uart_putstr_sync(OsVersion);
    uart_putstr_sync("\n");
    uart_putstr_sync("platform : ");
#if (OS_HARDWARE_PLATFORM == OS_RISCV64)
    uart_putstr_sync("riscv64\n");
#else 
    uart_putstr_sync("unkown platform\n");
#endif

    uart_putstr_sync("cpuType : ");
#if (OS_CPU_TYPE == OS_RV64_VIRT)
    uart_putstr_sync("qemu virt\n");
#else 
    uart_putstr_sync("unkown board\n");
#endif

    uart_putstr_sync("endian : ");
#if (OS_BYTE_ORDER == OS_LITTLE_ENDIAN)
    uart_putstr_sync("little endian\n");
#else
    uart_putstr_sync("big endian\n");
#endif

    uart_putstr_sync("max core: 0x");
    itoa_self(OS_MAX_CORE_NUM,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("clk frequence : 0x");
    itoa_self(OS_SYS_CLOCK,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");


    uart_putstr_sync("heap mem start : 0x");
    itoa_self(OS_MEM_FSC_PT_ADDR,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync("heap mem size : 0x");
    itoa_self(OS_MEM_FSC_PT_SIZE,ibuf);
    if(ibuf[0]==0) 
    {
        ibuf[0]='0';
        ibuf[1]=0;
    }
    uart_putstr_sync(ibuf);
    uart_putstr_sync("\n");

    uart_putstr_sync(">> ");

}
void console_error_handle()
{
    uart_putstr_sync("\e[91msorry,command i don't know please input 'help' to get information\n\e[39m");
    uart_putstr_sync(">> ");
}
void console_hwi_test()
{
    if(PRT_HwiDisable(UART0_IRQ) != OS_OK) 
    {
        uart_putstr_sync("disable hwi uart failed\n");
        return;
    }

    uart_putstr_sync("======================disable hwi uart start=============\n");
    uart_putstr_sync("from now on, your uart int is disabled for 5s\n");
    PRT_ClkDelayMs(5000);
    uart_putstr_sync("======================disable hwi uart end===============\n");
    while(PRT_HwiEnable(UART0_IRQ) != OS_OK) ;
    uart_putstr_sync(">> ");
}

void console_help()
{
    uart_putstr_sync("\e[92msystem      :  \e[95m show system information \n\e[39m");
    
    uart_putstr_sync("\e[92mhwitest_1   :  \e[95m test hwi uapi enable and disable \n\e[39m");

    uart_putstr_sync("\e[92mhelp        :  \e[95m get command help \n\e[39m");

    uart_putstr_sync("\e[92mhwitest_2   :  \e[95m test hwi uapi hook add \n\e[39m");

    uart_putstr_sync("\e[92mhwitest_3   :  \e[95m test hwi uapi hook del \n\e[39m");

    uart_putstr_sync("\e[92mhook_exc    :  \e[95m test exc uapi hook add \n\e[39m");

    uart_putstr_sync("\e[92mexc_trig    :  \e[95m test exc uapi trigger excption (maybe you should add exc hook that will let it more good when exc)\n\e[39m");

    uart_putstr_sync("\e[92mcreat_thd   :  \e[95m create a thread that will print hello,world once a second but not resume\n\e[39m");
    
    uart_putstr_sync("\e[92mresum_thd   :  \e[95m resume the thread hello,world\n\e[39m");

    uart_putstr_sync("\e[92msuspd_thd   :  \e[95m suspend the thread hello,world\n\e[39m");
    
    uart_putstr_sync("\e[92mdele_thd    :  \e[95m delete hello world thread\n\e[39m");

    uart_putstr_sync("\e[92mdelay_thd   :  \e[95m delay console  thread for 5s\n\e[39m");
    
    uart_putstr_sync("\e[92mlock_test  :  \e[95m use hello_world thread to lock sched lock for 5s (hello_world thread must be resumed before)\n\e[39m");
    uart_putstr_sync(">> ");

}



char* get_param(char* str,char** next)
{
   size_t left = 0;
   size_t right = 0;
   size_t i =0;
   while(str[i] !=0 && str[i] == ' ') i++;
   if(str[i] == 0)
   {
        *next = NULL;
        return NULL;
   }
   left = i;
   while(str[i] !=0 && str[i] != ' ') i++;
   if(str[i] == 0)
   {
        *next = NULL;
        return (str+left);
   }
   right = i;
   str[right] = '\0';
   if(str[right+1] != '\0')
   {
        *next = (str+right+1);
   }
   else
   {
        *next = NULL;
   }
   return (str + left);
}

INLINE enum C_TYPE console_parse(char* str,size_t size)
{
    char* command = NULL;
    char* next_str = str;
    command = get_param(next_str,&next_str);
    if(command == NULL)
    {
        return CONSOLE_ERROR;    
    }
    if(strcmp(command,"system\n")==0 || strcmp(command,"sys\n") ==0) 
    {
        return CONSOLE_BASE_INFO;
    }
    if(strcmp(command,"hwitest_1\n")==0)
    {
        return CONSOLE_HWI_TEST;
    }
    if(strcmp(command,"help\n")==0)
    {
        return CONSOLE_HELP;
    }
    if(strcmp(command,"hwitest_2\n")==0)
    {
        return CONSOLE_HWI_TEST_2;
    }
    if(strcmp(command,"hwitest_3\n")==0)
    {
        return CONSOLE_HWI_TEST_3;
    }
    if(strcmp(command,"hook_exc\n")==0)
    {
        return CONSOLE_EXC_TEST_1;
    }
    if(strcmp(command,"exc_trig\n")==0)
    {
        return CONSOLE_EXC_TEST_2;
    }
    if(strcmp(command,"creat_thd\n")==0)
    {
        return CONSOLE_HELLO_WORLD;
    }
    if(strcmp(command,"resum_thd\n")==0)
    {
        return CONSOLE_HELLO_WORLD_RS;
    }
    if(strcmp(command,"suspd_thd\n")==0)
    {
        return CONSOLE_HELLO_WORLD_SUSPEND;
    }
    if(strcmp(command,"dele_thd\n")==0)
    {
        return CONSOLE_DEL_HELLO_WORLD;
    }
    if(strcmp(command,"delay_thd\n")==0)
    {
        return CONSOLE_DELAY_HELLO_WORLD;
    }
    if(strcmp(command,"lock_test\n")==0)
    {
        return CONSOLE_LOCK_TEST;
    }
    return CONSOLE_ERROR;
}
TskHandle taskPid;
int want_lock =0;
void thread_lock()
{
    want_lock = 1;
}

void thread_hello(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4)
{
    while(1)
    {
        uart_putstr_sync("hello,world!\n");
        PRT_ClkDelayMs(1000);
        if(want_lock !=0)
        {
            PRT_TaskLock();
            uart_putstr_sync("=====on lock for 5 s in hello wordl thread======\n");
            uart_putstr_sync("=====high priority thread console thread can't load======\n");
            PRT_ClkDelayMs(5000);
            uart_putstr_sync("===unlock ======\n");
            want_lock = 0;
            PRT_TaskUnlock();
        }
    }
}
static int hello_world =0;
void console_hello_world()
{
    
    struct TskInitParam para;
    para.taskEntry = thread_hello;
    para.taskPrio = 30;
    para.stackSize = 0x1000;
    para.name ="test thread";
    para.stackAddr = 0;
    para.args[0] = 0;
    para.args[1] = 0;
    para.args[2] = 0;
    para.args[3] = 0;
    if(hello_world ==1 )
    {
        uart_putstr_sync("do not create repeate one!\n");
        uart_putstr_sync(">> ");
        return;
    }
    hello_world =1 ;
    if(PRT_TaskCreate(&taskPid,&para) != OS_OK)
    {
        uart_putstr_sync("create task fail!\n");
    }
    else
    {
        uart_putstr_sync("create task success!\n");
    }
    uart_putstr_sync(">> ");
}

void console_hello_word_rsm()
{
    if(hello_world != 1)
    {
        uart_putstr_sync("do not resume one that is not created!\n");
        uart_putstr_sync(">> ");
        return;
    }
    if(PRT_TaskResume(taskPid) != OS_OK)
    {
        uart_putstr_sync("resume task fail!\n");
    }
    else 
    {
        uart_putstr_sync("resume task success!\n");
    }
    uart_putstr_sync(">> ");
}


void console_hello_word_Suspend()
{
    if(hello_world != 1)
    {
        uart_putstr_sync("do not suspend one that is not created!\n");
        uart_putstr_sync(">> ");
        return;
    }
    if(PRT_TaskSuspend(taskPid) != OS_OK)
    {
        uart_putstr_sync("suspend task fail!\n");
    }
    else 
    {
        uart_putstr_sync("suspend task success!\n");
    }
    uart_putstr_sync(">> ");
}

void console_del_hello_world()
{
    if(hello_world == 0)
    {
        uart_putstr_sync("hello world don't create\n");
        uart_putstr_sync(">> ");
        return;
    }
    if(PRT_TaskDelete(taskPid) != OS_OK)
    {
        uart_putstr_sync("del task fail!\n");
    }
    hello_world=0;
    uart_putstr_sync("del task success!\n");
    uart_putstr_sync(">> ");
}

void console_delay_hello_world()
{
    if(PRT_TaskDelay(50) != OS_OK)
    {
         uart_putstr_sync("delay console task fail!\n");
    }
    uart_putstr_sync("delay console task for 5 s success!\n");
    uart_putstr_sync(">> ");
}

void console_handle(char *str)
{
    size_t size = strlen(str);
    enum C_TYPE ret = console_parse(str,size);
    switch(ret)
    {
        case CONSOLE_LOCK_TEST:
        thread_lock();
        break;
        case CONSOLE_DELAY_HELLO_WORLD:
        console_delay_hello_world();
        break;
        case CONSOLE_DEL_HELLO_WORLD:
        console_del_hello_world();
        break;
        case CONSOLE_HELLO_WORLD_SUSPEND:
        console_hello_word_Suspend();
        break;
        case CONSOLE_HELLO_WORLD_RS:
        console_hello_word_rsm();
        break;
        case CONSOLE_HELLO_WORLD:
        console_hello_world();
        break;
        case CONSOLE_EXC_TEST_1:
        console_exc_test();
        break;
        case CONSOLE_EXC_TEST_2:
        console_exc_trig();
        break;
        case CONSOLE_HWI_TEST_2:
        console_hwi_test_2();
        break;
        case CONSOLE_HWI_TEST_3:
        console_hwi_test_3();
        break;
        case CONSOLE_BASE_INFO:
        console_base_info();
        break;
        case CONSOLE_HWI_TEST:
        console_hwi_test();
        break;
        case CONSOLE_HELP:
        console_help();
        break;
        case CONSOLE_ERROR:
        default:
            console_error_handle();
        break;
    }
}
