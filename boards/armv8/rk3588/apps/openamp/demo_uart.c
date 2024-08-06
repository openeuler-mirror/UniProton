#include "prt_hwi.h"
#include "prt_clk.h"
#include "prt_task.h"
#include "hal_base.h"
#include "print.h"

#define TEST_UART UART0
#define TEST_UART_INT 363

static const struct HAL_UART_DEV uart_dev =
    {
        .pReg = TEST_UART,
        .sclkID = CLK_UART0,
};

static struct HAL_UART_CONFIG uart_config = {
    .baudRate = 115200,
    .dataBit = UART_DATA_8B,
    .stopBit = UART_ONE_STOPBIT,
    .parity = UART_PARITY_DISABLE,
};

static void UART_ShowPrompt()
{
    U8 prompt[] = "\r\nUniProton # ";
    HAL_UART_SerialOut(TEST_UART, prompt, strlen(prompt));
}

static void UART_Isr(uintptr_t para)
{
    uint32_t count = HAL_UART_GetRfl(TEST_UART);
    for (int i = 0; i < count; i++)
    {
        U8 ch;
        if (HAL_UART_SerialIn(TEST_UART, &ch, 1) == 1)
        {
            if (ch == '\r' || ch == '\t')
            {
                UART_ShowPrompt();
                continue;
            }

            if (ch >= 32 && ch <= 126)
            {
                HAL_UART_SerialOutChar(TEST_UART, ch);
            }
        }
    }
}

static U32 UART_IsrInit()
{
    U32 ret;
    ret = PRT_HwiSetAttr(TEST_UART_INT, 0, OS_HWI_MODE_ENGROSS);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiCreate(TEST_UART_INT, (HwiProcFunc)UART_Isr, 0);
    if (ret != OS_OK)
    {
        return ret;
    }

    ret = PRT_HwiEnable(TEST_UART_INT);
    if (ret != OS_OK)
    {
        return ret;
    }

    HAL_UART_EnableIrq(TEST_UART, UART_IER_RDI);
    return OS_OK;
}

static void UART_DrvInit()
{
    HAL_PINCTRL_SetIOMUX(GPIO_BANK4, GPIO_PIN_A3 | GPIO_PIN_A4, PIN_CONFIG_MUX_FUNC10);
    HAL_UART_Init(&uart_dev, &uart_config);
}

static void UART_ShowMessage()
{
    U8 str[60];
    sprintf((char *)str, "\r\n===== UART Baudrate = %d bps =====\r\n", uart_config.baudRate);
    HAL_UART_SerialOut(TEST_UART, str, strlen(str));
}

void UART_Demo()
{
    PRT_Printf("UART Demo\n");

    UART_IsrInit();
    UART_DrvInit();
    UART_ShowMessage();

    while (1)
    {
        PRT_TaskDelay(10);
    }
}
