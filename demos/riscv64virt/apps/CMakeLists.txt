if (APP STREQUAL "hello_world")
	add_subdirectory(hello_world)
elseif (APP STREQUAL "task-switch")
	add_subdirectory(rhealstone)
elseif (APP STREQUAL "task-preempt")
	add_subdirectory(rhealstone)
elseif (APP STREQUAL "message-latency")
	add_subdirectory(rhealstone)
elseif (APP STREQUAL "deadlock-break")
	add_subdirectory(rhealstone)
elseif (APP STREQUAL "semaphore-shuffle")
	add_subdirectory(rhealstone)
elseif (APP STREQUAL "shell")
	add_subdirectory(shell)
elseif (APP MATCHES "^UniProton_test_posix.*")
	add_subdirectory(libc-test)
endif()
