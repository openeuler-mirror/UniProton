set(GDBSTUB_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/rsp_utils.c
        ${CMAKE_CURRENT_SOURCE_DIR}/gdbstub_common.c
        ${CMAKE_CURRENT_SOURCE_DIR}/ringbuffer.c
)

if(${CONFIG_OS_ARCH_X86_64})
    list(APPEND GDBSTUB_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64/gdbstub.c
    )
elseif (${CONFIG_OS_ARCH_ARMV8})
    list(APPEND GDBSTUB_SRC
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/arm64/gdbstub.c
    )
endif()

add_library(gdbstub_src OBJECT ${GDBSTUB_SRC})

if(${CONFIG_OS_ARCH_X86_64})
    target_include_directories(gdbstub_src PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/arch/x86_64
    )
elseif (${CONFIG_OS_ARCH_ARMV8})
    target_include_directories(gdbstub_src PUBLIC
            ${CMAKE_CURRENT_SOURCE_DIR}/arch/arm64
    )
endif()

target_include_directories(gdbstub_src PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS gdbstub_src)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

