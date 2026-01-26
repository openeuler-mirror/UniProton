if (NOT ${APP} STREQUAL "UniProton_uros_demo")
    return()
endif()

set(BUILD_APP "UniProton_uros_demo")
set(ALL_SRC
    addTwoInt_server.c
    helloworld_sub.c
    helloworld_pub.c
    turtlesim_action.c
    walk_maze.c
)

add_library(urosTest OBJECT ${ALL_SRC})