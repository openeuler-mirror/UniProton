set(ARM_COMMON_SOURCE arm_udelay.c
                      arm_mdelay.c)

add_library(arm_common OBJECT ${ARM_COMMON_SOURCE})

target_include_directories(arm_common PUBLIC 
    ${FS_BASE_DIR}/include
    ${HOME_PATH}/src/libc/litelibc/internal/include
    ${HOME_PATH}/src/libc/litelibc/include
    ${HOME_PATH}/src/libc/musl/src/include
    ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS arm_common)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)