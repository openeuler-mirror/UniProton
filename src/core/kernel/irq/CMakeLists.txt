file(GLOB_RECURSE KERNEL_IRQ_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

if(NOT "${CONFIG_OS_OPTION_SMP}")
    list(REMOVE_ITEM KERNEL_IRQ_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/prt_irq_minor.c
    )
endif()

add_library(kernel_irq OBJECT ${KERNEL_IRQ_SRC})

target_include_directories(kernel_irq PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS kernel_irq)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)