################交叉编译#####################
#cross-compilation config
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)


set(CPU_TYPE "$ENV{CPU_TYPE}" )
set(PLAM_TYPE "$ENV{PLAM_TYPE}" )
set(LIB_TYPE "$ENV{LIB_TYPE}"  )
set(SYSTEM "$ENV{SYSTEM}" )
set(CORE "$ENV{CORE}" )
set(LIB_RUN_TYPE "$ENV{LIB_RUN_TYPE}" )
set(BUILD_DIR "$ENV{BUILD_TMP_DIR}" ) #version id
set(OBJCOPY_PATH "$ENV{OBJCOPY_PATH}" ) #OBJCOPY_PATH
set(COMPILE_MODE "$ENV{COMPILE_MODE}" )
set(CC_TYPE "$ENV{CC_TYPE}" ) 
set(TOOLCHAIN_DIR "$ENV{HCC_PATH}") #该路径应该是外部传入,指向编译工具路径


# 设置gcc_arm64编译器公共的编译选项
set(CC_OPT_LEVEL "-Os")
set(CC_WARN_FLAGS "-Wformat-signedness")
set(CC_MD_DEPENDENT_FLAGS "-Wl,--build-id=none")
# 初始化为空，不能删除这句
set(CC_OPT_FLAGS "")
set(CC_SEC_FLAGS "${CC_SEC_FLAGS} -fno-PIE")
# 待确认的编译选项，暂时没进行分类，待处理
set(CC_OTHER_FLAGS "-fno-builtin -fno-dwarf2-cfi-asm -mcmodel=large -fomit-frame-pointer -fzero-initialized-in-bss -fdollars-in-identifiers -ffunction-sections -fdata-sections -fno-aggressive-loop-optimizations -fno-optimize-strlen -fno-schedule-insns -fno-inline-small-functions -fno-inline-functions-called-once -fno-strict-aliasing -finline-limit=20 -mstrict-align -mlittle-endian -nostartfiles -funwind-tables")
set(CC_DEFINE_FLAGS "")

##compiler specified in /etc/profile
set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/aarch64-none-elf-gcc" CACHE PATH "arm-gcc C compiler" FORCE)
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_DIR}/aarch64-none-elf-gcc" CACHE PATH "arm-gcc ASM compiler" FORCE)

# 设置C和ASM相关的所有使用的编译选项
set(CMAKE_C_FLAGS "${CC_OPT_LEVEL} ${CC_OVERALL_FLAGS_COMMON} ${CC_WARN_FLAGS_COMMON} ${CC_WARN_FLAGS} ${CC_LANGUAGE_FLAGS_COMMON} ${CC_LANGUAGE_FLAGS} ${CC_CDG_FLAGS} ${CC_MD_DEPENDENT_FLAGS} ${CC_OPT_FLAGS} ${CC_SEC_FLAGS} ${CC_OTHER_FLAGS} ${CC_DEFINE_FLAGS_COMMON} ${CC_DEFINE_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} ${ASM_EXTRA_FLAGS}")
set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER> <FLAGS> <INCLUDES> -c <SOURCE> -o <OBJECT>")
file(STRINGS "$ENV{CONFIG_FILE_PATH}/defconfig" config_options REGEX "^CONFIG_OS_OPTION_POSIX" ENCODING "UTF-8")
foreach(config_option ${config_options})
    set(CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT} -D_GNU_SOURCE -D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS")
endforeach()
set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER> <FLAGS> <INCLUDES> -c <SOURCE> -o <OBJECT>")

set(CMAKE_LINKER "${TOOLCHAIN_DIR}/aarch64-none-elf-ld" CACHE STRING "" FORCE) 
set(CMAKE_AR "${TOOLCHAIN_DIR}/aarch64-none-elf-ar" CACHE STRING "" FORCE) 
set(CMAKE_C_LINK_FLAGS "-r ")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -r <TARGET> <OBJECTS>") 
