
if(${CONFIG_OS_OPTION_EVENT})
    add_subdirectory(event)
endif()

if(${CONFIG_OS_OPTION_QUEUE})
    add_subdirectory(queue)
endif()

add_subdirectory(sem)

