set(CMSIS_VERSION_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/cmsis_os.h)
file(READ ${CMSIS_VERSION_HEADER} CMSIS_VERSION_HEADER_CONTENT)
string(REGEX MATCH "#[ \t]*define[ \t]+CMSIS_OS_VER[ \t]+([12])" CMSIS_VERSION_MATCH "${CMSIS_VERSION_HEADER_CONTENT}")
if (NOT CMSIS_VERSION_MATCH)
    message(FATAL_ERROR "CMSIS_OS_VER must be defined as 1 or 2 in ${CMSIS_VERSION_HEADER}")
endif()
set(CMSIS_OS_VER ${CMAKE_MATCH_1})
set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS ${CMSIS_VERSION_HEADER})

set(CMSIS_SRC
    cmsis_uniproton.c
)

if (CMSIS_OS_VER EQUAL 1)
    list(APPEND CMSIS_SRC 1.0/cmsis_uniproton1.c)
else()
    list(APPEND CMSIS_SRC 2.0/cmsis_uniproton2.c)
endif()

add_library(cmsis OBJECT ${CMSIS_SRC})
target_include_directories(cmsis PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/1.0
    ${CMAKE_CURRENT_SOURCE_DIR}/2.0
    ${HOME_PATH}/src/arch/include
    ${HOME_PATH}/src/core/kernel/include
    ${HOME_PATH}/src/include/uapi
    ${CONFIG_FILE_PATH}
)

list(APPEND ALL_OBJECT_LIBRARYS cmsis)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)
