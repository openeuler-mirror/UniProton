set(CMAKE_ASM_COMPILER_ID "ARMASM")
add_library(prt_hw OBJECT prt_hw.S)
set_target_properties(prt_hw PROPERTIES OUTPUT_NAME "prt_hw") #对目标文件重命名
add_library(prt_hw_exc OBJECT prt_hw_exc.S)
set_target_properties(prt_hw_exc PROPERTIES OUTPUT_NAME "prt_hw_exc") #对目标文件重命名
add_library(prt_dispatch OBJECT prt_dispatch.S)
set_target_properties(prt_dispatch PROPERTIES OUTPUT_NAME "prt_dispatch") #对目标文件重命名
add_library(prt_vector OBJECT prt_vector.S)
set_target_properties(prt_vector PROPERTIES OUTPUT_NAME "prt_vector") #对目标文件重命名
add_library(prt_div64 OBJECT prt_div64.c)