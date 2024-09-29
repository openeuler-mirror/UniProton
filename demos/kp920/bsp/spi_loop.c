#include "spi.h"
#include "prt_hwi.h"
#include "prt_task.h"
#include "prt_gic_external.h"
#include "prt_typedef.h"

void IoWrite32(U32 value, U64 address)
{
    *(volatile U32 *)address = value;
}

U32 IoRead32(U64 address)
{
    return *(volatile U32 *)address;
}

/* 设置某GPIO电平，然后将该GPIO数据方向设为输出 */
void SpiSetGpioOut(U32 gpio, U32 level)
{
    if (gpio >= GPIO_MAX_NUM) {
        return;
    }
    // GPIO序号为0-31，通过寄存器1配置，否则通过寄存器1配置
    U64 base = GPIO0_REG_BASE;
    if (gpio >= GPIO_PER_REG_NUM) {
        base = GPIO1_REG_BASE;
    }
    // 取置位掩码
    U32 mask = 0x1U << (gpio % GPIO_PER_REG_NUM);
    // 修改GPIO电平，0x2_0112_0000 或 0x2_0113_0000
    U32 value = IoRead32(base + GPIO_SWPORT_DR_REG);
    value = (level == GPIO_LEVEL_LOW) ? (value & (~mask)) : (value | mask);
    IoWrite32(value, base + GPIO_SWPORT_DR_REG);
    // 修改GPIO输出方向，0x2_0112_0004 或 0x2_0113_0004
    value = IoRead32(base + GPIO_SWPORT_DDR_REG);
    value |= mask;
    IoWrite32(value, base + GPIO_SWPORT_DDR_REG);
}

static SpiAttr g_spiAttr = {
    .port = 0,
    .dev = 0,
    .loopBack = 1,
    .baud_rate = 1000000,
    .frameType = MOTOROLA,
    .frameLength = SPI_FRAME_LEN_8_BIT,
    .clockEdge = SPI_CLOCK_EDGE_UP,
    .clockPolarity = SPI_CLOCK_POLARITY_LOW
};

/* 配置GPIO */
static void SpiConfigGpio()
{
    IoWrite32(0, IOMUX_REG_BASE + GPIO38_REG);  // 0x2_0110_0064
    IoWrite32(3, IOMUX_REG_BASE + GPIO02_REG);  // 0x2_0110_007C
    IoWrite32(3, IOMUX_REG_BASE + GPIO03_REG);  // 0x2_0110_0080
    IoWrite32(3, IOMUX_REG_BASE + GPIO05_REG);  // 0x2_0110_0088
    IoWrite32(0, IOMUX_REG_BASE + GPIO41_REG);  // 0x2_0110_0070
}

static void SpiTpmInit(SpiAttr *spiAttr)
{
    // SPI撤销软复位，0x2_0107_0A64
    IoWrite32(0xFFFFFFFF, SUBCTRL_REG_BASE + SC_SPI_RESET_DREQ_REG);
    
    // CPLD使能，0x8000_0020
    IoWrite32(1, CPLD_BASE + ENABLE_SPI_OFFSET);

    // 选择从机设备，0x2_011A_0010
    IoWrite32(0x1U << spiAttr->dev, SPI_REG_BASE + SPI_SER_REG);

    // 配置并行复用，0x2_0107_2000
    IoWrite32(0x1U << spiAttr->dev, SUBCTRL_REG_BASE + SC_SPI_MUX_REG);

    SpiConfigGpio();
    SpiSetGpioOut(GPIO38, GPIO_LEVEL_HIGH);
    SpiSetGpioOut(GPIO41, GPIO_LEVEL_HIGH);

    // 配置采样周期，0x2_011A_00F0
    IoWrite32(0x5, SPI_REG_BASE + RX_SAMPLE_DLY_REG);
}

static void spi_disable()
{
    IoWrite32(0x0, SPI_REG_BASE + SPI_SSIENR_REG);
    return;
}

static void spi_enable()
{
    IoWrite32(0x1, SPI_REG_BASE + SPI_SSIENR_REG);
    return;
}

static void spi_loop_set(U8 enable)
{
    spi_disable();

    SpiCtrlr0 spiCtrlr0 = {0};
    spiCtrlr0.value = IoRead32(SPI_REG_BASE + SPI_CTRLR0_REG);
    spiCtrlr0.bits.srl = enable;
    IoWrite32(spiCtrlr0.value, SPI_REG_BASE + SPI_CTRLR0_REG);

    spi_enable();
    return;
}

static void SpiConfigCtrl(SpiAttr *spiAttr)
{
    // 关闭spi
    spi_disable();

    // 设置波特率，当前时钟频率 = 内部时钟/分频系数
    U32 freq_div = 0;
    freq_div = SPI_SYSTEM_CLOCK / spiAttr->baud_rate;
    if (0 != (freq_div & 0x1))
    {
        freq_div++;
    }

    // 填充波特率 
    IoWrite32(freq_div, SPI_REG_BASE + SPI_BAUDR_REG);

    // 关闭所有中断
    IoWrite32(0x0, SPI_REG_BASE + SPI_IMR_REG);

    // 设置SPI CTRLR0控制寄存器
    SpiCtrlr0 spiCtrlr0 = {0};
    spiCtrlr0.value = IoRead32(SPI_REG_BASE + SPI_CTRLR0_REG);
    spiCtrlr0.bits.srl = 0x0;
    spiCtrlr0.bits.frf = 0x0;
    spiCtrlr0.bits.dfs32 = 0x1f;
    spiCtrlr0.bits.scpol = spiAttr->clockPolarity;
    spiCtrlr0.bits.scph = spiAttr->clockEdge;
    IoWrite32(spiCtrlr0.value, SPI_REG_BASE + SPI_CTRLR0_REG);

    //spi使能 
    spi_enable();
}

static void spi_clear_tx_rx_fifo()
{
    U32 val;

    spi_loop_set(LOOP_ENABLE);
    IoWrite32(0x1, SPI_REG_BASE + SPI_SER_REG);

    // 清空tx_fifo
    while(1) {
        val = IoRead32(SPI_REG_BASE + SPI_TXFLR_REG);
        if(val == 0) {
            break;
        }
    }

    // 清空rx_fifo
    while(1) {
        val = IoRead32(SPI_REG_BASE + SPI_RXFLR_REG);
        if(val == 0) {
            break;
        }

        val = IoRead32(SPI_REG_BASE + SPI_DR0_REG);
    }

    spi_loop_set(LOOP_DISABLE);
}

void spi_init()
{
    SpiAttr *spiAttr = &g_spiAttr;

    SpiTpmInit(spiAttr);

    SpiConfigCtrl(spiAttr);

    spi_clear_tx_rx_fifo();
}

U32 spi_get_recv_fifo_status()
{
    U32 tmp = 0;

    // spi接收后状态，判断接收fifo是否为空
    tmp = IoRead32(SPI_REG_BASE + SPI_SR_REG);
    if (0x0 == (tmp & 0x8)) {
        return SPI_RECV_FIFO_EMPTY;
    } else {
        return SPI_RECV_FIFO_NOT_EMPTY;
    } 
}

void spi_tx_rx(U8 port, U8 cs, U32 *tx_buf, U32 tx_len, U32 *rx_buf, U32 rx_len)
{
    SpiCtrlr0 spiCtrlr0 = {0};
    U8 frame_len;
    U32 tmp = 0;

    // spi去使能
    spi_disable();

    // 设置为双工模式
    spiCtrlr0.value = IoRead32(SPI_REG_BASE + SPI_CTRLR0_REG);
    spiCtrlr0.bits.tmod = 0;
    frame_len = spiCtrlr0.bits.dfs32;
    IoWrite32(spiCtrlr0.value, SPI_REG_BASE + SPI_CTRLR0_REG);

    spi_enable();

    // 设置从机,spi模块不发出片选信号
    IoWrite32((0x0 << cs), SPI_REG_BASE + SPI_SER_REG);
    SpiSetGpioOut(GPIO38, GPIO_LEVEL_LOW);

    // 往TX FIFO中填写要发送的数据
    for (int i = 0; i < tx_len; i++) {
        tmp = tx_buf[i];
        IoWrite32(tmp, SPI_REG_BASE + SPI_DR0_REG);
    }

    // 设置从机, spi模块发出片选信号, 将TX FIFO中的数据发送出去
    IoWrite32((0x1 << cs), SPI_REG_BASE + SPI_SER_REG);
    PRT_TaskDelay(100);

    // 等待数据到来
    for(int wait = 0; wait < 1000; wait++) {
        PRT_TaskDelay(10);
        if(SPI_RECV_FIFO_NOT_EMPTY == spi_get_recv_fifo_status()) {
            break;
        }
    }

    // 没有收到数据
    if(SPI_RECV_FIFO_EMPTY == spi_get_recv_fifo_status()) {
        LOG_HERE(0x80);
    }

    // 从RX FIFO获取读到的数据
    for(int i = 0; i < tx_len; i++) {
        tmp = IoRead32(SPI_REG_BASE + SPI_DR0_REG);
        rx_buf[i] = tmp & FRAME_LEN_TO_MASK(frame_len);
    }

    SpiSetGpioOut(GPIO38, GPIO_LEVEL_HIGH);
}

void spi_loop()
{
    spi_init();

    // 设立回环标记位
    spi_loop_set(LOOP_ENABLE);

    U32 tx_buf[LOOP_TEST_FRAME_NUM] = {0x55555555, 0xaaaaaaaa, 0x5a5a5a5a, 0xa5a5a5a5};
    U32 rx_buf[LOOP_TEST_FRAME_NUM] = {0};

    spi_tx_rx(0, 0, tx_buf, LOOP_TEST_FRAME_NUM, rx_buf, LOOP_TEST_FRAME_NUM);

    for (int i = 0; i < LOOP_TEST_FRAME_NUM; i++) {
        if (tx_buf[i] & FRAME_LEN_TO_MASK(SPI_FRAME_LEN_8_BIT) != rx_buf[i])
        {
            spi_loop_set(LOOP_DISABLE);
            return;
        }
        
    }
    spi_loop_set(LOOP_DISABLE);
}