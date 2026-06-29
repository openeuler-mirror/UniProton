add_library_ex(prt_mem.c)

# 内存算法宏控选择（以算法名命名，便于后续新增 bestfit 等）：
#   CONFIG_OS_MEM_ARITH_TLSF=y -> 编译 TLSF 算法 (tlsf/prt_tlsfmem.c 胶水 + tlsf/prt_tlsf_core.c 算法源码)
#   CONFIG_OS_MEM_ARITH_BESTFIT_LITTLE=y -> 编译 bestfit_little 算法
#   CONFIG_OS_MEM_ARITH_BESTFIT=y -> 编译 bestfit 算法
#   否则（缺省）               -> 编译原有 UniProton FSC 算法 (fsc/prt_fscmem.c)
# 每个算法各自实现自己的初始化 (OsFscMemInit / OsTlsfMemInit) 并填充 g_memArithAPI
# 分发表；公共分派层 OsMemInit（prt_mem.c）按本宏选用对应算法的初始化，
# 板级 prt_config.c 的 OsMemConfigReg 只调用 OsMemInit，无需感知算法差异。
# CONFIG_OS_MEM_ARITH_TLSF 由顶层 CMakeLists.txt 的 import_kconfig 从 defconfig 注入。
if("${CONFIG_OS_MEM_ARITH_TLSF}" STREQUAL "y")
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/config/config
    )
    add_library_ex(tlsf/prt_tlsfmem.c)
    add_library_ex(tlsf/prt_tlsf_core.c)
elseif("${CONFIG_OS_MEM_ARITH_BESTFIT_LITTLE}" STREQUAL "y")
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/config/config
        ${CMAKE_CURRENT_SOURCE_DIR}/bestfit_little
        ${CMAKE_CURRENT_SOURCE_DIR}/bestfit_little/compat
        ${CMAKE_CURRENT_SOURCE_DIR}/slab
    )
    add_library_ex(bestfit_little/prt_bestfit_littlemem.c)
    add_library_ex(bestfit_little/prt_bestfit_little_memory.c)
    add_library_ex(bestfit_little/prt_bestfit_little_heap.c)
    if("${CONFIG_OS_MEM_SLAB_EXTENTION}" STREQUAL "y")
        add_library_ex(slab/prt_slab.c)
        add_library_ex(slab/prt_slabmem.c)
    endif()
elseif("${CONFIG_OS_MEM_ARITH_BESTFIT}" STREQUAL "y")
    include_directories(
        ${PROJECT_SOURCE_DIR}/src/config/config
        ${CMAKE_CURRENT_SOURCE_DIR}/bestfit
        ${CMAKE_CURRENT_SOURCE_DIR}/bestfit/compat
        ${CMAKE_CURRENT_SOURCE_DIR}/slab
    )
    add_library_ex(bestfit/prt_bestfitmem.c)
    add_library_ex(bestfit/prt_bestfit_memory.c)
    add_library_ex(bestfit/prt_bestfit_multipledlinkhead.c)
    if("${CONFIG_OS_MEM_SLAB_EXTENTION}" STREQUAL "y")
        add_library_ex(slab/prt_slab.c)
        add_library_ex(slab/prt_slabmem.c)
    endif()
else()
    add_library_ex(fsc/prt_fscmem.c)
endif()
