set(SRC main.c rpmsg_backend.c rpmsg_service.c
    demo_gpio.c
    demo_timer.c
    demo_uart.c
)

add_library(openamp OBJECT ${SRC})
