/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_timedwrlock(pthread_rwlock_t *rwlock)
 *
 *	Under no circumstances shall the function fail with a timeout if the lock can be 
 *	acquired immediately. The abs_timeout parameter need not be checked if the lock 
 *	can be immediately acquired.
 *
 * Steps:
 * 1.  Main thread create a thread.
 * 2.  Child thread lock 'rwlock' for writing with pthread_rwlock_timedwrlock(), 
 *	should not fail with timeout
 * 3.  The child thread unlocks the 'rwlock' and exit.
 * 4.  Main thread create another thread.
 * 5.  The child thread lock 'rwlock' for write, with pthread_rwlock_timedwrlock(), 
 *	specifying a 'abs_timeout'. The thread sleeps untile 'abs_timeout' expires.
 * 6.  The thread call pthread_rwlock_timedwrlock(). Should not get ETIMEDOUT.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

#define TIMEOUT 1

static int pthread_rwlock_timedwrlock_5_1_thread_state; 
static int pthread_rwlock_timedwrlock_5_1_thread_currsec1;
static int pthread_rwlock_timedwrlock_5_1_thread_expired;

/* pthread_rwlock_timedwrlock_5_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_timedwrlock_5_1_fn_wr_1(void *arg)
{ 
	pthread_rwlock_timedwrlock_5_1_thread_state = ENTERED_THREAD;
	struct timespec abs_timeout;
	int rc;
	pthread_rwlock_t rwlock = {0};

	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("thread1: Error at pthread_rwlock_init\n");
		return PTS_UNRESOLVED ;
	}

	pthread_rwlock_timedwrlock_5_1_thread_currsec1 = time(NULL);

	/* Absolute time, not relative. */
	abs_timeout.tv_sec = pthread_rwlock_timedwrlock_5_1_thread_currsec1 + TIMEOUT;
	abs_timeout.tv_nsec = 0;	

	printf("thread1: attempt timed lock for writing\n");	
	rc = pthread_rwlock_timedwrlock(&rwlock, &abs_timeout);
	if(rc  == ETIMEDOUT)
	{
		printf("thread1: timer pthread_rwlock_timedwrlock_5_1_thread_expired\n");
		pthread_rwlock_timedwrlock_5_1_thread_expired = 1;
	}
	else if(rc == 0)
	{
		printf("thread1: acquired write lock\n");
		pthread_rwlock_timedwrlock_5_1_thread_expired = 0;
		printf("thread1: unlock write lock\n");
		if(pthread_rwlock_unlock(&rwlock) != 0)
		{
			printf("thread1: failed to release write lock\n");	
			return PTS_UNRESOLVED ;
		}
	}
	else
	{
		printf("thread1: Error in pthread_rwlock_timedwrlock().\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("thread1: Error at pthread_rwlockattr_destroy()");
		return PTS_UNRESOLVED ;
	}	
	pthread_rwlock_timedwrlock_5_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
static void* pthread_rwlock_timedwrlock_5_1_fn_wr_2(void *arg)
{ 
	pthread_rwlock_timedwrlock_5_1_thread_state = ENTERED_THREAD;
	struct timespec abs_timeout;
	int rc;
	pthread_rwlock_t rwlock = {0};

	if(pthread_rwlock_init(&rwlock, NULL) != 0)
	{
		printf("thread2: Error at pthread_rwlock_init\n");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_timedwrlock_5_1_thread_currsec1 = time(NULL);

	/* Absolute time, not relative. */
	abs_timeout.tv_sec = pthread_rwlock_timedwrlock_5_1_thread_currsec1 - TIMEOUT;
	abs_timeout.tv_nsec = 0;	
	
	printf("thread2: attempt timed lock for writing\n");	
	rc = pthread_rwlock_timedwrlock(&rwlock, &abs_timeout);
	if(rc  == ETIMEDOUT)
	{
		printf("thread2: timer pthread_rwlock_timedwrlock_5_1_thread_expired\n");
		pthread_rwlock_timedwrlock_5_1_thread_expired = 1;
	}
	else if(rc == 0)
	{
		printf("thread2: acquired write lock\n");
		pthread_rwlock_timedwrlock_5_1_thread_expired = 0;
		printf("thread2: unlock write lock\n");
		if(pthread_rwlock_unlock(&rwlock) != 0)
		{
			printf("thread2: failed to release write lock\n");
			return PTS_UNRESOLVED ;
		}
	}
	else
	{
		printf("thread2: Error in pthread_rwlock_timedwrlock().\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_rwlock_destroy(&rwlock) != 0)
	{
		printf("thread2: Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED ;
	}	
	pthread_rwlock_timedwrlock_5_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}

int pthread_rwlock_timedwrlock_5_1()
{
	int cnt = 0;
	
	pthread_t thread1, thread2;
	
	pthread_rwlock_timedwrlock_5_1_thread_state = NOT_CREATED_THREAD;
	printf("main: create thread1\n");
	if(pthread_create(&thread1, NULL, pthread_rwlock_timedwrlock_5_1_fn_wr_1, NULL) != 0)
	{
		printf("Error when creating thread1\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */
	/* we expect thread1 NOT to block */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedwrlock_5_1_thread_state !=3 && cnt++ < 5); 
		
	if(pthread_rwlock_timedwrlock_5_1_thread_state == 3)
	{
		/* the child thread does not block, check the time pthread_rwlock_timedwrlock_5_1_thread_expired or not */
		if(pthread_rwlock_timedwrlock_5_1_thread_expired == 1)
		{
			printf("Test FAILED: thread1 got ETIMEOUT when get the lock\n");
			return PTS_FAIL;
		}
	}
	else if(pthread_rwlock_timedwrlock_5_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread1 blocked\n");
		return PTS_FAIL;
	}
	else
	{
		printf("Unexpected state for thread1 %d\n", pthread_rwlock_timedwrlock_5_1_thread_state);
		return PTS_UNRESOLVED;
	}
	
	if(pthread_join(thread1, NULL) != 0)
	{
		printf("Error when joining thread1\n");
		return PTS_UNRESOLVED;
	}

	pthread_rwlock_timedwrlock_5_1_thread_state = ENTERED_THREAD;
	printf("main: create thread2\n");
	if(pthread_create(&thread2, NULL, pthread_rwlock_timedwrlock_5_1_fn_wr_2, NULL) != 0)
	{
		printf("Error when creating thread2\n");
		return PTS_UNRESOLVED;
	}
	
	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */
	
	/* we expect thread2 NOT to block */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedwrlock_5_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
		
	if(pthread_rwlock_timedwrlock_5_1_thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time pthread_rwlock_timedwrlock_5_1_thread_expired or not */
		if(pthread_rwlock_timedwrlock_5_1_thread_expired == 1)
		{
			printf("Test FAILED: thread2 got ETIMEOUT\n");
			return PTS_FAIL;
		}
	}
	else if(pthread_rwlock_timedwrlock_5_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread2 blocked\n");
		return PTS_FAIL;
	}
	else
	{
		printf("Unexpected state for thread2 %d\n", pthread_rwlock_timedwrlock_5_1_thread_state);
		return PTS_UNRESOLVED;
	}
	
	if(pthread_join(thread2, NULL) != 0)
	{
		printf("Error when join thread2\n");
		return PTS_UNRESOLVED;
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
