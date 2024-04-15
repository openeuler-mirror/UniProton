if(NOT "${CONFIG_OS_OPTION_POSIX}")
    RETURN()
endif()

message(${CMAKE_SYSTEM_PROCESSOR})

# 所有litelibc中适配的.c文件
file(GLOB_RECURSE LITELIBC_SOURCE litelibc/src/*.c litelibc/internal/*.c)

file(GLOB_RECURSE LITELIBC_DEP_SIGNAL_SOURCE litelibc/src/signal/*.c litelibc/src/exit/*)
if (NOT CONFIG_OS_OPTION_POSIX_SIGNAL)
list(REMOVE_ITEM LITELIBC_SOURCE
    ${LITELIBC_DEP_SIGNAL_SOURCE}
)
endif()

# 依赖VFS的接口
if(NOT "${CONFIG_OS_OPTION_NUTTX_VFS}")
list(REMOVE_ITEM LITELIBC_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/close.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/dup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/dup2.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/gethostname.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/read.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/write.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/unlink.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/readlink.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/unistd/rmdir.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/fopen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/fclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/freopen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/rename.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/tmpfile.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/popen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/stdio/pclose.c
)

#移除litelibc中的stat模块，只有定义CONFIG_OS_OPTION_NUTTX_VFS才加入stat模块
file(GLOB_RECURSE LITELIBC_STAT_SOURCE litelibc/src/stat/*.c)
list(REMOVE_ITEM LITELIBC_SOURCE ${LITELIBC_STAT_SOURCE})

#移除litelibc中的fcntl模块，只有定义CONFIG_OS_OPTION_NUTTX_VFS才加入fcntl模块
file(GLOB_RECURSE LITELIBC_FCNTL_SOURCE litelibc/src/fcntl/*.c)
list(REMOVE_ITEM LITELIBC_SOURCE ${LITELIBC_FCNTL_SOURCE})

endif()

# 需要编译的musl的.c文件
set(MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/sleep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/usleep.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/c_locale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/__lctrans.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal/libc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/vsprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strlen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/wctomb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/wcrtomb.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/memchr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strcat.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strrchr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strtok_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strncasecmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/prng/rand_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/prng/rand.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/__stack_chk_fail.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/__environ.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/getenv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/htonl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/htons.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/ntohs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/ntohl.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/inet_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/inet_aton.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/inet_ntoa.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/inet_pton.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/inet_ntop.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/if_freenameindex.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal/syscall_ret.c
    )

if(NOT "${CONFIG_OS_OPTION_PROXY}")
    list(APPEND  MULS_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/select/poll.c)
endif()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/${CMAKE_SYSTEM_PROCESSOR}/arch.mak)
list(APPEND  MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_getres_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_gettime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_nanosleep_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/clock_settime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/ctime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/difftime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/ftime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/gettimeofday_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/gmtime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/localtime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/mktime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/nanosleep_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_mutex_timedlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_rwlock_timedrdlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_rwlock_timedwrlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/timer_gettime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/timer_settime32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/timespec_get_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/utime_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/sem_timedwait_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/ctime32_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/localtime32_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/gmtime32_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/thrd_sleep_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_timedjoin_np_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/pthread_cond_timedwait_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/cnd_timedwait_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/mtx_timedlock_time32.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/futimesat_time32.c
)
endif()

if (${CPU_TYPE} STREQUAL "hi3093" AND "${CONFIG_OS_SUPPORT_CXX}" STREQUAL "y")
    list(APPEND  MULS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/misc/getauxval.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/snprintf.c
    )
endif()

set(MUSL_LOCALE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/wcscoll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/strxfrm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/wcsxfrm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/strcoll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/__mo_lookup.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/setlocale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/locale_map.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/newlocale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/duplocale.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/freelocale.c
)

set(MUSL_MULTIBYTE_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbsrtowcs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbstowcs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbrtowc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbsrtowcs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbstowcs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/wctob.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/btowc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/internal.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbtowc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbsinit.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbsnrtowcs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/wcsrtombs.c
)

if (${CPU_TYPE} STREQUAL "x86_64")
    list(APPEND  MULS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/sprintf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/snprintf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/vsprintf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/sscanf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/vsscanf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/multibyte/mbsinit.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/vfscanf.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal/intscan.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal/shgetc.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/getenv.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/__environ.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/htons.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/htonl.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/ntohs.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/network/ntohl.c
    )
endif()

## nuttx vfs 依赖
if (${CPU_TYPE} STREQUAL "kp920")
    list(APPEND  MULS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/asprintf.c
    )
endif()

file(GLOB_RECURSE MUSL_TIME_SOURCE musl/src/time/*.c)
file(GLOB MUSL_THREAD_SOURCE musl/src/thread/*.c)
file(GLOB MUSL_IPC_SOURCE musl/src/ipc/*.c)

file(GLOB MUSL_CTYPE_SOURCE musl/src/ctype/*.c)

list(APPEND MULS_SOURCE ${MUSL_CTYPE_SOURCE})

# 当前不支持功能，不提供该接口
list(REMOVE_ITEM MUSL_TIME_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/__map_file.c
    # 当前不支持文件系统
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/utime.c
    # 当前不支持文件系统、环境变量获取
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/getdate.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/strftime.c
)
list(REMOVE_ITEM MUSL_THREAD_SOURCE
    # 当前不支持syscall、tls、vmclock
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/clone.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/synccall.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/syscall_cp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/tls.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/vmlock.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__set_thread_area.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__syscall_cp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__timedwait.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__tls_get_addr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__unmapself.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/thread/__wait.c
    )

list(REMOVE_ITEM MUSL_IPC_SOURCE
    # 当前不支持文件系统
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/ipc/ftok.c
)

# 查找math编译文件  
file(GLOB MUSL_MATH_SOURCE musl/src/math/*.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/math/${CMAKE_SYSTEM_PROCESSOR})
    file(GLOB MUSL_MATH_ARCH_SOURCE musl/src/math/${CMAKE_SYSTEM_PROCESSOR}/*.[csS])
    foreach(tmp_one_source ${MUSL_MATH_ARCH_SOURCE})
        string(REPLACE "/${CMAKE_SYSTEM_PROCESSOR}/" "/" tmp_replace_source ${tmp_one_source})
        string(REPLACE ".s" ".c" tmp_replace_source ${tmp_replace_source})
        string(REPLACE ".S" ".c" tmp_replace_source ${tmp_replace_source})
        list(FIND MUSL_MATH_SOURCE ${tmp_replace_source} tmp_find_result)
        if(${tmp_find_result} GREATER_EQUAL 0)
            list(REMOVE_ITEM MUSL_MATH_SOURCE ${tmp_replace_source})
        endif()
    endforeach()
    list(APPEND MUSL_MATH_SOURCE ${MUSL_MATH_ARCH_SOURCE})
endif()

file(GLOB MUSL_FENV_SOURCE musl/src/fenv/*.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/fenv/${CMAKE_SYSTEM_PROCESSOR}/)
    file(GLOB MUSL_FENV_ARCH_SOURCE musl/src/fenv/${CMAKE_SYSTEM_PROCESSOR}/*.[csS])
    list(REMOVE_ITEM MUSL_FENV_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/fenv/fenv.c)
    list(APPEND MUSL_MATH_SOURCE ${MUSL_FENV_ARCH_SOURCE})
endif()

# 查找ctype编译文件
file(GLOB MUSL_CTYPE_SOURCE musl/src/ctype/*.c)
# 查找multibyte编译文件
file(GLOB MUSL_MULTIB_SOURCE musl/src/multibyte/*.c)
# 查找regex编译文件
file(GLOB MUSL_REGEX_SOURCE musl/src/regex/*.c)
# 查找prng编译文件
file(GLOB MUSL_PRNG_SOURCE musl/src/prng/*.c)
# 查找string编译文件
file(GLOB MUSL_STRING_SOURCE musl/src/string/*.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/${CMAKE_SYSTEM_PROCESSOR})
    file(GLOB MUSL_STRING_ARCH_SOURCE musl/src/string/${CMAKE_SYSTEM_PROCESSOR}/*.[csS])
    foreach(tmp_one_source ${MUSL_STRING_ARCH_SOURCE})
        string(REPLACE "/${CMAKE_SYSTEM_PROCESSOR}/" "/" tmp_replace_source ${tmp_one_source})
        string(REPLACE ".s" ".c" tmp_replace_source ${tmp_replace_source})
        string(REPLACE ".S" ".c" tmp_replace_source ${tmp_replace_source})
        list(FIND MUSL_STRING_SOURCE ${tmp_replace_source} tmp_find_result)
        if(${tmp_find_result} GREATER_EQUAL 0)
            list(REMOVE_ITEM MUSL_STRING_SOURCE ${tmp_replace_source})
        endif()
    endforeach()
    list(APPEND MUSL_STRING_SOURCE ${MUSL_STRING_ARCH_SOURCE})
endif()

# 适配stdlib
file(GLOB MUSL_STDLIB_SOURCE musl/src/stdlib/*.c)
# stdlib 依赖
list(APPEND  MUSL_STDLIB_SOURCE
    musl/src/stdio/sprintf.c
    musl/src/stdio/__uflow.c
    musl/src/stdio/__toread.c
    musl/src/stdio/__stdio_exit.c
    musl/src/stdio/__lockfile.c
    musl/src/stdio/ofl.c
    musl/src/internal/shgetc.c
    musl/src/internal/intscan.c
    musl/src/internal/floatscan.c
    musl/src/string/wcschr.c
    musl/src/string/wcslen.c
)

# 适配stdio
if (NOT (${CPU_TYPE} STREQUAL "hi3093"))
    file(GLOB MUSL_STDIO_SOURCE musl/src/stdio/*.c)
endif()

list(APPEND MUSL_STDIO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/lseek.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/temp/__randname.c
)
list(REMOVE_ITEM MUSL_STDIO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/__fopen_rb_ca.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/stdin.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/stdout.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/stderr.c
    
)

if (${CONFIG_OS_OPTION_PROXY})
list(REMOVE_ITEM MUSL_STDIO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fwrite.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fputs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fopen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fclose.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fread.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/freopen.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fputs.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fgets.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/feof.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/ferror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/ungetc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/getc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fseek.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/ftell.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/clearerr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/getc_unlocked.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fflush.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fileno.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/fscanf.c
)
endif()

if (${CPU_TYPE} STREQUAL "x86_64" OR ${CPU_TYPE} STREQUAL "uvp")
list(REMOVE_ITEM MUSL_STDIO_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/perror.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/printf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/puts.c
)
endif()

# 适配unistd
set(MULS_UNISTD_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/ualarm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/lseek.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/posix_close.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/misc/realpath.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/setenv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/putenv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/unsetenv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/alarm.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/setpgrp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/ttyname.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/ttyname_r.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/unistd/tcgetpgrp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal/procfdname.c
)

# 需要编译的signal相关.c文件
file(GLOB MUSL_SIGNAL_SOURCE musl/src/signal/*.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/setjmp/${CMAKE_SYSTEM_PROCESSOR})
    file(GLOB MUSL_SIGNAL_ARCH_SOURCE musl/src/setjmp/${CMAKE_SYSTEM_PROCESSOR}/*.[csS])
    foreach(tmp_one_source ${MUSL_SIGNAL_ARCH_SOURCE})
        string(REPLACE "/${CMAKE_SYSTEM_PROCESSOR}/" "/" tmp_replace_source ${tmp_one_source})
        string(REPLACE ".s" ".c" tmp_replace_source ${tmp_replace_source})
        string(REPLACE ".S" ".c" tmp_replace_source ${tmp_replace_source})
        list(FIND MUSL_SIGNAL_SOURCE ${tmp_replace_source} tmp_find_result)
        if(${tmp_find_result} GREATER_EQUAL 0)
            list(REMOVE_ITEM MUSL_SIGNAL_SOURCE ${tmp_replace_source})
        endif()
    endforeach()
    list(APPEND MUSL_SIGNAL_SOURCE ${MUSL_SIGNAL_ARCH_SOURCE})
endif()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/${CMAKE_SYSTEM_PROCESSOR}/arch.mak)
    list(APPEND  MUSL_SIGNAL_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/getitimer_time32.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/compat/time32/setitimer_time32.c
    )
endif()
# 查找setjmp编译文件 - signal依赖
file(GLOB MUSL_SETJMP_SOURCE musl/src/setjmp/*.c)
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/setjmp/${CMAKE_SYSTEM_PROCESSOR})
    file(GLOB MUSL_SETJMP_ARCH_SOURCE musl/src/setjmp/${CMAKE_SYSTEM_PROCESSOR}/*.[csS])
    foreach(tmp_one_source ${MUSL_SETJMP_ARCH_SOURCE})
        string(REPLACE "/${CMAKE_SYSTEM_PROCESSOR}/" "/" tmp_replace_source ${tmp_one_source})
        string(REPLACE ".s" ".c" tmp_replace_source ${tmp_replace_source})
        string(REPLACE ".S" ".c" tmp_replace_source ${tmp_replace_source})
        list(FIND MUSL_SETJMP_SOURCE ${tmp_replace_source} tmp_find_result)
        if(${tmp_find_result} GREATER_EQUAL 0)
            list(REMOVE_ITEM MUSL_SETJMP_SOURCE ${tmp_replace_source})
        endif()
    endforeach()
    list(APPEND MUSL_SETJMP_SOURCE ${MUSL_SETJMP_ARCH_SOURCE})
endif()


# lua依赖的相关.c文件
if(${CPU_TYPE} STREQUAL "rk3568_jailhouse" OR ${CPU_TYPE} STREQUAL "rk3588" OR ${CPU_TYPE} STREQUAL "e2000q")
list(APPEND MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strcmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strchrnul.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strchr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/memmove.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/memset.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strpbrk.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/memcpy.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/stdio/snprintf.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strspn.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/memcmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/strcoll.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strstr.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/prng/random.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/__environ.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/env/getenv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/locale/localeconv.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/setjmp/aarch64/setjmp.s
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/setjmp/aarch64/longjmp.s
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strncmp.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/string/strcspn.c
    )
endif()

file(GLOB MUSL_EXIT_SOURCE musl/src/exit/*.c)
list(REMOVE_ITEM MUSL_EXIT_SOURCE
    # 暂不支持文件系统，删除该接口
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/exit/assert.c
)
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "arm")
    list(APPEND MUSL_EXIT_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/exit/arm/__aeabi_atexit.c
    )
endif()

# 解决缺少__libc_malloc
list(APPEND MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/malloc/libc_calloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/malloc/replaced.c
)

list(APPEND MULS_SOURCE
    ${MUSL_TIME_SOURCE}
    ${MUSL_THREAD_SOURCE}
    ${MUSL_MATH_SOURCE}
    ${MUSL_FENV_SOURCE}
    ${MUSL_STDLIB_SOURCE}
    ${MUSL_IPC_SOURCE}
    ${MUSL_REGEX_SOURCE}
    ${MUSL_STRING_SOURCE}
    ${MUSL_PRNG_SOURCE}
    ${MUSL_LOCALE_SOURCE}
    ${MUSL_SETJMP_SOURCE}
    ${MUSL_CTYPE_SOURCE}
    ${MUSL_MULTIBYTE_SOURCE}
    ${MUSL_STDIO_SOURCE}
    ${MULS_UNISTD_SOURCE}
)

if(${CONFIG_OS_OPTION_NUTTX_VFS})

# 添加musl中的stat模块
file(GLOB MULS_STAT_SOURCE musl/src/stat/*.c)
# 添加musl中的fcntl模块
file(GLOB MULS_FCNTL_SOURCE musl/src/fcntl/*.c)
# 添加musl中的dirent模块
file(GLOB MULS_DIRENT_SOURCE musl/src/dirent/*.c)

list(APPEND MULS_SOURCE
    ${MULS_STAT_SOURCE}
    ${MULS_FCNTL_SOURCE}
    ${MULS_DIRENT_SOURCE}
)
endif()

if(${CONFIG_OS_OPTION_POSIX_SIGNAL})
list(APPEND MULS_SOURCE
    ${MUSL_SIGNAL_SOURCE}
    ${MUSL_EXIT_SOURCE}
)
endif()

if(${CONFIG_OS_SUPPORT_LIBXML2})
    list(APPEND MULS_SOURCE
        ${MUSL_MULTIB_SOURCE}
    )
endif()

if(${CONFIG_OS_OPTION_LOCALE})
    list(APPEND MULS_SOURCE
        ${MUSL_LOCALE_SOURCE}
        ${MUSL_MULTIBYTE_SOURCE}
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/time/strftime.c
    )
    list(REMOVE_ITEM LITELIBC_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/src/time/strftime.c
    )
endif()

if(${CONFIG_OS_OPTION_STM32F407_UART})
list(APPEND MULS_SOURCE
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/termios/cfgetospeed.c
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/termios/cfsetospeed.c
    )
endif()

if(${CONFIG_OS_OPTION_PROXY})
    set(MUSL_FS_SOURCE
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/dirent/opendir.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/dirent/readdir.c
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/dirent/closedir.c
        )
    list(APPEND MULS_SOURCE    ${MUSL_FS_SOURCE})
endif()

list(REMOVE_DUPLICATES MULS_SOURCE)
list(REMOVE_DUPLICATES LITELIBC_SOURCE)
add_library(musllibc OBJECT ${MULS_SOURCE} ${LITELIBC_SOURCE})
target_compile_options(musllibc PUBLIC 
        -Wno-bad-function-cast
        -Wno-unused-value
        -Wno-switch-default
        -Wno-float-equal
        -Wno-unused-but-set-variable
        -Wno-discarded-qualifiers
        -Wno-implicit-fallthrough
        -Wno-cast-function-type
        -Wno-float-conversion
        -Wno-unknown-pragmas
        -Wno-maybe-uninitialized
        -Wno-address
        -include ${CONFIG_FILE_PATH}/prt_buildef.h
        )
if (${CONFIG_OS_POSIX_SET_TZDST})
    target_compile_options(musllibc PUBLIC -DPOSIX_SET_TZDST)
endif()
target_include_directories(musllibc PUBLIC 
        ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/internal/include
        ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/include
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/include
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/${CMAKE_SYSTEM_PROCESSOR}
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/arch/generic
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/src/internal
        ${CMAKE_CURRENT_SOURCE_DIR}/musl/include
)

list(APPEND ALL_OBJECT_LIBRARYS musllibc)
set(ALL_OBJECT_LIBRARYS ${ALL_OBJECT_LIBRARYS} CACHE STRING INTERNAL FORCE)

file(GLOB_RECURSE LITELIBC_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/include/*.h)
STRING(REPLACE 
       "${CMAKE_CURRENT_SOURCE_DIR}/litelibc/"
       "${CMAKE_CURRENT_SOURCE_DIR}/musl/" 
       DEL_INCLUDE
       "${LITELIBC_INCLUDE}")

add_custom_target(del_include_file ALL)
add_custom_command(TARGET del_include_file
                   POST_BUILD
                   COMMAND rm -rf ${DEL_INCLUDE}
                   DEPENDS ${DEL_INCLUDE}
)

install(DIRECTORY
    ${CMAKE_CURRENT_SOURCE_DIR}/musl/include
    ${CMAKE_CURRENT_SOURCE_DIR}/litelibc/include
    DESTINATION libc/
)
