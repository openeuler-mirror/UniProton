/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_timedrdlock(pthread_rwlock_t *pthread_rwlock_timedrdlock_2_1_rwlock)
 *
 *	The timeout shall expire when the absolute time specified by abs_timeout passes, 
 *	as measured by the clock on which timeouts are based (that is, when the
 *	value of that clock equals or exceeds abs_timeout), or if the absolute time 
 *	specified by abs_timeout has already been passed at the time of the call.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_timedrdlock_2_1_rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'pthread_rwlock_timedrdlock_2_1_rwlock' for writing with pthread_rwlock_rdlock()
 * 3.  Create a child thread, specify a 'abs_timeout' as being the current time _minus_
 *     a timeout value of 1. (this ensures that the abs_timeout has already passed)
 * 4.  The thread lock 'pthread_rwlock_timedrdlock_2_1_rwlock' for reading, using pthread_rwlock_timedrdlock(). Should
 *	get an ETIMEOUT error. 
 */

#define _XOPEN_SOURCE 600

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

/* pthread_rwlock_timedrdlock_2_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 1

static pthread_rwlock_t pthread_rwlock_timedrdlock_2_1_rwlock;
static int pthread_rwlock_timedrdlock_2_1_thread_state; 
static int pthread_rwlock_timedrdlock_2_1_currsec1, pthread_rwlock_timedrdlock_2_1_currsec2;
static int pthread_rwlock_timedrdlock_2_1_expired;

static void* pthread_rwlock_timedrdlock_2_1_fn_rd(void *arg)
{ 
	struct timespec abs_timeout;
	int rc;
	pthread_rwlock_timedrdlock_2_1_thread_state = ENTERED_THREAD;

	pthread_rwlock_timedrdlock_2_1_currsec1 = time(NULL);

	/* Absolute time, not relative. */
	abs_timeout.tv_sec = pthread_rwlock_timedrdlock_2_1_currsec1 - TIMEOUT;
	abs_timeout.tv_nsec = 0;	
	
	printf("thread: attempt timed read-lock\n");	
	rc = pthread_rwlock_timedrdlock(&pthread_rwlock_timedrdlock_2_1_rwlock, &abs_timeout);
	if(rc  == ETIMEDOUT)
	{
		printf("thread: timed read-lock correctly pthread_rwlock_timedrdlock_2_1_expired\n");
		pthread_rwlock_timedrdlock_2_1_expired = 1;
	}
	else if(rc == 0)
	{
		printf("thread: acquired read-lock\n");
		pthread_rwlock_timedrdlock_2_1_expired = 0;
		printf("thread: unlock read lock\n");
		if(pthread_rwlock_unlock(&pthread_rwlock_timedrdlock_2_1_rwlock) != 0)
		{
			printf("thread: failed to release lock\n");
			return PTS_UNRESOLVED ;
		}
	}
	else
	{
		printf("Error in pthread_rwlock_timedrdlock(), error code:%d.\n", rc);
		return PTS_UNRESOLVED ;
	}
	
	/* Get time after the mutex timed out in locking. */
	pthread_rwlock_timedrdlock_2_1_currsec2 = time(NULL);
	pthread_rwlock_timedrdlock_2_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int pthread_rwlock_timedrdlock_2_1()
{
	int cnt = 0;
	pthread_t thread1;
	
	pthread_rwlock_timedrdlock_2_1_expired = 0;

	if(pthread_rwlock_init(&pthread_rwlock_timedrdlock_2_1_rwlock, NULL) != 0)
	{
		printf("Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_timedrdlock_2_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired write lock\n");
	
	printf("main: create thread\n");
	pthread_rwlock_timedrdlock_2_1_thread_state = NOT_CREATED_THREAD;
	if(pthread_create(&thread1, NULL, pthread_rwlock_timedrdlock_2_1_fn_rd, NULL) != 0)
	{
		printf("Error creating thread1\n");
		return PTS_UNRESOLVED;
	}
	
	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */

	/* We expect the thread _NOT_ to block, and instead time out */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedrdlock_2_1_thread_state !=EXITING_THREAD && cnt++ < 5); 

	if(pthread_rwlock_timedrdlock_2_1_thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time pthread_rwlock_timedrdlock_2_1_expired or not */
		if(pthread_rwlock_timedrdlock_2_1_expired != 1)
		{
			printf("Test FAILED: abs_timeout should expire\n");
			return PTS_FAIL ;
		}
	}
	else if(pthread_rwlock_timedrdlock_2_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread blocked even when the timer pthread_rwlock_timedrdlock_2_1_expired\n");
		return PTS_FAIL ;
	}
	else
	{
		printf("Unexpected thread state %d\n", pthread_rwlock_timedrdlock_2_1_thread_state);
		return PTS_UNRESOLVED ;
	}
	
	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_timedrdlock_2_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED;
	}
	
	if(pthread_join(thread1, NULL) != 0)
	{
		printf("main: Error at pthread_join()\n");
		return PTS_UNRESOLVED;
	}

	if(pthread_rwlock_destroy(&pthread_rwlock_timedrdlock_2_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
