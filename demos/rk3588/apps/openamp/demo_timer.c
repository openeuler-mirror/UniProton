#include "prt_hwi.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "hal_base.h"
#include "print.h"

#define TEST_TIMER TIMER1
#define TEST_TIMER_INT 322

static void TIMER_Isr()
{
    static U64 clk_old = 0;
    U64 clk_now;

    HAL_TIMER_ClrInt(TEST_TIMER);
    clk_now = PRT_ClkGetCycleCount64();
    if (clk_old != 0)
    {
        PRT_Printf("timer isr = %llu us\n", PRT_ClkCycle2Us(clk_now - clk_old));
    }
    clk_old = clk_now;
}

static U32 TIMER_IsrInit()
{
    U32 ret = PRT_HwiSetAttr(TEST_TIMER_INT, 0, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return ret;
    }
    
    ret = PRT_HwiCreate(TEST_TIMER_INT, (HwiProcFunc)TIMER_Isr, 0);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiEnable(TEST_TIMER_INT);
    if (ret != OS_OK)
    {
        return ret;
    }

    return OS_OK;
}

static U32 TIMER_DrvInit()
{
    HAL_TIMER_Stop(TEST_TIMER);
    HAL_TIMER_Init(TEST_TIMER, TIMER_FREE_RUNNING);
    HAL_TIMER_SetCount(TEST_TIMER, 12000000);
    HAL_TIMER_Start_IT(TEST_TIMER);

    return OS_OK;
}

void TIMER_Demo()
{
    PRT_Printf("TIMER Demo\n");

    TIMER_IsrInit();
    TIMER_DrvInit();

    while(1)
    {
        PRT_TaskDelay(10);
    }
}
