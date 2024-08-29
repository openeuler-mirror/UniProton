file(GLOB_RECURSE DRIVER_GIC ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(driver_gic OBJECT ${DRIVER_GIC})

target_include_directories(driver_gic PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${HOME_PATH}/src/libc/litelibc/internal/include
        ${HOME_PATH}/src/libc/litelibc/include
        ${HOME_PATH}/src/libc/musl/src/include
        ${HOME_PATH}/src/libc/musl/include)

if("${CONFIG_INTERNAL_OS_GIC_VER}" STREQUAL "gicv600")
    target_include_directories(driver_gic PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/gicv600
    )
endif()

list(APPEND ALL_OBJECT_LIBRARYS driver_gic)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)