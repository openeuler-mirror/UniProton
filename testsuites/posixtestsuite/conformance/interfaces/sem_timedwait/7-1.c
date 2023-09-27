/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content 
 * of this license, see the COPYING file at the top level of this 
 * source tree.
 */

/* This tests case will open a locked semaphore.  The time will tick 5 times
 * until the absolute time passes.  The sempahore will unlock, then the 
 * sem_timedwait call will immediately lock again.
 */

#define _XOPEN_SOURCE 600

#include <stdio.h>
// #include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <strings.h>
#include <semaphore.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"


#define TEST "7-1"
#define FUNCTION "sem_timedwait"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "


int sem_timedwait_7_1(){

    struct timespec ts;
    sem_t mysemp;

	if ( sem_init (&mysemp, 0, 0) == -1 ) {
		perror(ERROR_PREFIX "sem_init");
		return PTS_UNRESOLVED;
	}

	// TODO: 支持timeout后不需要
	// ts.tv_sec = time(NULL);
	if (clock_gettime(CLOCK_REALTIME, &ts) != 0) {
		perror("clock_gettime()");
		return PTS_UNRESOLVED;
	}

	ts.tv_sec += 1;
	ts.tv_nsec=0;

	while (sem_timedwait(&mysemp, &ts) == -1 )
	{
		if (errno == ETIMEDOUT)  {
			puts("TEST PASSED");
			sem_destroy(&mysemp);
			return PTS_PASS;
		} else {
			puts("TEST FAILED");
			return PTS_FAIL;
		}
	return PTS_UNRESOLVED;
	}
return PTS_UNRESOLVED;
}
