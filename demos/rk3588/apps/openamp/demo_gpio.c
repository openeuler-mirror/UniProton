#include "prt_hwi.h"
#include "prt_clk.h"
#include "hal_base.h"
#include "print.h"

#define TEST_GPIO_INT 309

static bool keyPressed = FALSE;
void HAL_GPIO_IRQDispatch(eGPIO_bankId bank, uint32_t pin)
{
    switch (bank)
    {
    case GPIO_BANK0:
        keyPressed = TRUE;
        break;
    case GPIO_BANK1:
        break;
    case GPIO_BANK2:
        break;
    case GPIO_BANK3:
        break;
    case GPIO_BANK4:
        break;
    default:
        break;
    }
}

void GPIO_Isr(uintptr_t para)
{
    HAL_GPIO_IRQHandler(GPIO0, GPIO_BANK0);
}

static U32 GPIO_IsrInit()
{
    U32 ret;
    ret = PRT_HwiSetAttr(TEST_GPIO_INT, 10, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiCreate(TEST_GPIO_INT, (HwiProcFunc)GPIO_Isr, 0);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiEnable(TEST_GPIO_INT);

    return OS_OK;
}

static void GPIO_DrvInit()
{
    /**
     * KEY5: GPIO0_C4
     * LED1: GPIO0_C5
     */
    HAL_PINCTRL_SetParam(GPIO_BANK0, GPIO_PIN_C4 | GPIO_PIN_C5,
                         PIN_CONFIG_MUX_FUNC0 | PIN_CONFIG_PUL_UP);

    HAL_GPIO_SetPinDirection(GPIO0, GPIO_PIN_C4, GPIO_IN);
    HAL_GPIO_SetPinDirection(GPIO0, GPIO_PIN_C5, GPIO_OUT);

    HAL_GPIO_SetIntType(GPIO0, GPIO_PIN_C4, GPIO_INT_TYPE_EDGE_FALLING);
    HAL_GPIO_EnableIRQ(GPIO0, GPIO_PIN_C4);
}

void GPIO_Demo()
{
    PRT_Printf("GPIO Demo\n");

    GPIO_IsrInit();
    GPIO_DrvInit();

    while (1)
    {
        if (keyPressed == TRUE)
        {
            PRT_Printf("key pressed at %lld s\n", PRT_ClkGetCycleCount64() / 24000000);
            keyPressed = FALSE;
            PRT_TaskDelay(10);
        }

        if (HAL_GPIO_GetPinLevel(GPIO0, GPIO_PIN_C4) == GPIO_LOW)
        {
            HAL_GPIO_SetPinLevel(GPIO0, GPIO_PIN_C5, GPIO_HIGH);
        }
        else
        {
            HAL_GPIO_SetPinLevel(GPIO0, GPIO_PIN_C5, GPIO_LOW);
        }
    }
}
