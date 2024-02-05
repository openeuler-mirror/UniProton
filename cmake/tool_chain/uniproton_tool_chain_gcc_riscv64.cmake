################交叉编译#####################
#cross-compilation config
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR riscv64)

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

##compiler specified in /etc/profile
set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/riscv64-unknown-elf-gcc" CACHE PATH "gcc C compiler" FORCE)
set(CMAKE_ASM_COMPILER "${TOOLCHAIN_DIR}/riscv64-unknown-elf-gcc" CACHE PATH "gcc ASM compiler" FORCE)

set(CMAKE_ASM_COMPILE_OBJECT "<CMAKE_ASM_COMPILER>  -g -nostdlib -nostartfiles -nodefaultlibs -fno-builtin <FLAGS> <INCLUDES> -c <SOURCE> -o <OBJECT>")
set(CMAKE_C_COMPILE_OBJECT "<CMAKE_C_COMPILER>  -mcmodel=medany  -g ${COMPILE_WARING_FLAG} -static -nostdlib -nostartfiles -nodefaultlibs  -fno-builtin -fno-PIE   -fomit-frame-pointer -fzero-initialized-in-bss   -fno-common -fno-stack-protector  -funsigned-char -fno-PIC  <FLAGS> <INCLUDES> -c <SOURCE> -o <OBJECT>")


file(STRINGS "$ENV{CONFIG_FILE_PATH}/defconfig" config_options REGEX "^CONFIG_OS_OPTION_POSIX" ENCODING "UTF-8")
foreach(config_option ${config_options})
    set(CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT} -D_GNU_SOURCE -D_POSIX_THREADS -D_POSIX_THREAD_PRIORITY_SCHEDULING -D_POSIX_PRIORITY_SCHEDULING -D_POSIX_TIMERS -D_POSIX_CPUTIME -D_POSIX_THREAD_CPUTIME -D_POSIX_MONOTONIC_CLOCK -D_POSIX_TIMEOUTS -D_POSIX_CLOCK_SELECTION -D_POSIX_THREAD_PRIO_PROTECT -D_UNIX98_THREAD_MUTEX_ATTRIBUTES -D_POSIX_READER_WRITER_LOCKS -Wno-error=dangling-pointer -Wno-error=array-parameter -Wno-error=incompatible-pointer-types -Wno-error=pointer-to-int-cast -Wno-error=int-to-pointer-cast -march=rv64gc -mabi=lp64d")
endforeach()

set(CMAKE_LINKER "${TOOLCHAIN_DIR}/riscv64-unknown-elf-ld" CACHE STRING "" FORCE)
set(CMAKE_AR "${TOOLCHAIN_DIR}/riscv64-unknown-elf-ar" CACHE STRING "" FORCE)
set(CMAKE_C_LINK_FLAGS "-r ")
set(CMAKE_C_ARCHIVE_CREATE "<CMAKE_AR> -r <TARGET> <OBJECTS>")
