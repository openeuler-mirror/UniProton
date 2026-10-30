set(ALL_SRC pinocchio_test.cc ${HOME_PATH}/demos/sd3403/apps/openamp/openarm_gravity.c)
set_source_files_properties(${ALL_SRC} PROPERTIES COMPILE_OPTIONS "-O2")
add_library(pintest OBJECT ${ALL_SRC})
