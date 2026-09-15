set(COAL_DIR ${HOME_PATH}/demos/sd3403/component/coal-2.4.5)
file(GLOB COAL_SRCS ${COAL_DIR}/src/*.cpp ${COAL_DIR}/src/*/*.cpp)
list(FILTER COAL_SRCS EXCLUDE REGEX "serialization|octomap|octree|broadphase|mesh_loader")
set(ALL_SRC coal_test.cc ${COAL_SRCS})
# -nostdinc environments hit boost/cstdint.hpp without a prior <stdint.h>;
# force-include it for the third-party sources
set_source_files_properties(${COAL_SRCS} PROPERTIES COMPILE_OPTIONS "-O2;-include;stdint.h")
set_source_files_properties(coal_test.cc PROPERTIES COMPILE_OPTIONS "-O2")
add_library(coaltest OBJECT ${ALL_SRC})
target_include_directories(coaltest PUBLIC ${COAL_DIR}/include)
