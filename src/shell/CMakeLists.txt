
message("======== Compile Shell dir  ========")

file(GLOB_RECURSE SHELL_SOURCE full/*.c adapter/*.c)

add_library(libshell OBJECT ${SHELL_SOURCE})

target_include_directories(libshell PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/full/include
        ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include
)

list(APPEND ALL_OBJECT_LIBRARYS libshell)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/full/include/shell.h
    ${CMAKE_CURRENT_SOURCE_DIR}/full/include/show.h
    DESTINATION shell/include/
)

install(FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/los_base.h
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/los_event.h
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/los_printf.h
    ${CMAKE_CURRENT_SOURCE_DIR}/adapter/include/console.h
    DESTINATION shell/adapter/include/
)