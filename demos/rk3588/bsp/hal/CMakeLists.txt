set(SRCS
    hal_base.c
    hal_canfd.c
    hal_cru.c
    hal_cru_rk3588.c
    hal_gpio.c
    hal_i2c.c
    hal_pinctrl.c
    hal_spi.c
    hal_timer.c
    hal_uart.c
    )
    
add_library(hal OBJECT ${SRCS})
