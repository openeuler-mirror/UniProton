add_library_ex(prt_mem.c)

# 内存算法宏控选择（以算法名命名，便于后续新增 bestfit/bestfit_little/slab 等）：
#   CONFIG_OS_MEM_ARITH_TLSF=y -> 编译 TLSF 算法 (tlsf/prt_tlsfmem.c 胶水 + tlsf/prt_tlsf_core.c 算法源码)
#   否则（缺省）               -> 编译原有 UniProton FSC 算法 (fsc/prt_fscmem.c)
# 每个算法各自实现自己的初始化 (OsFscMemInit / OsTlsfMemInit) 并填充 g_memArithAPI
# 分发表；公共分派层 OsMemInit（prt_mem.c）按本宏选用对应算法的初始化，
# 板级 prt_config.c 的 OsMemConfigReg 只调用 OsMemInit，无需感知算法差异。
# CONFIG_OS_MEM_ARITH_TLSF 由顶层 CMakeLists.txt 的 import_kconfig 从 defconfig 注入。
if("${CONFIG_OS_MEM_ARITH_TLSF}" STREQUAL "y")
    add_library_ex(tlsf/prt_tlsfmem.c)
    add_library_ex(tlsf/prt_tlsf_core.c)
else()
    add_library_ex(fsc/prt_fscmem.c)
endif()
