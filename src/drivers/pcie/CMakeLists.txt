file(GLOB_RECURSE DRIVERS_PCIE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(drivers_pcie OBJECT ${DRIVERS_PCIE_SRC})

target_include_directories(drivers_pcie PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS drivers_pcie)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)