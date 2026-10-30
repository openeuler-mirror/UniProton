set(ALL_SRC eigen_openarm_test.cc ${HOME_PATH}/demos/sd3403/apps/openamp/openarm_gravity.c)
# The sd3403 demo compiles everything with -O0 by default, which makes Eigen
# template timing meaningless; benchmark sources must use -O2.
set_source_files_properties(${ALL_SRC} PROPERTIES COMPILE_OPTIONS "-O2")
add_library(eigentest OBJECT ${ALL_SRC})
