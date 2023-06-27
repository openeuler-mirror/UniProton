include_directories(include)
add_definitions(-D_GNU_SOURCE
                -D_POSIX_THREADS
                -D_POSIX_THREAD_PRIORITY_SCHEDULING
                -D_POSIX_TIMERS
                -D_POSIX_CPUTIME
                -D_POSIX_THREAD_CPUTIME
                -D_POSIX_MONOTONIC_CLOCK
                -D_POSIX_TIMEOUTS
                -D_POSIX_CLOCK_SELECTION
                -D_POSIX_THREAD_PRIO_PROTECT
                -D_UNIX98_THREAD_MUTEX_ATTRIBUTES
                -D_POSIX_READER_WRITER_LOCKS
                -D_POSIX_THREAD_PROCESS_SHARED
                -D_POSIX_BARRIERS
                -D_POSIX_SPORADIC_SERVER)

add_subdirectory(conformance)
