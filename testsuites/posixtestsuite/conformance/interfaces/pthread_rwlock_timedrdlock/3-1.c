/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_timedrdlock(pthread_rwlock_t *pthread_rwlock_timedrdlock_3_1_rwlock)
 *
 *  The function shall apply a read lock to the read-write lock referenced by
 *  pthread_rwlock_timedrdlock_3_1_rwlock as in the pthread_rwlock_rdlock(). However, if the lock cannot be
 *  acquired with out waiting for other threads to unlock the lock, this wait
 *  shall be terminated when the specified timeout expires.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_timedrdlock_3_1_rwlock' with pthread_rwlock_init()
 * 2.  Main thread lock 'pthread_rwlock_timedrdlock_3_1_rwlock' for reading with pthread_rwlock_rdlock()
 * 3.  Create a child thread, the thread lock 'pthread_rwlock_timedrdlock_3_1_rwlock' for reading, 
 *     using pthread_rwlock_timedrdlock(), should get read lock. Thread unlocks 'pthread_rwlock_timedrdlock_3_1_rwlock'.
 * 4.  Main thread unlock 'pthread_rwlock_timedrdlock_3_1_rwlock'
 * 5.  Main thread lock 'pthread_rwlock_timedrdlock_3_1_rwlock' for writing
 * 6.  Create child thread to lock 'pthread_rwlock_timedrdlock_3_1_rwlock' for reading, 
 *     using pthread_rwlock_timedrdlock, should block
 *     but when the timer expires, the wait will be terminated
 * 7.  Main thread unlock 'pthread_rwlock_timedrdlock_3_1_rwlock'
 */

/* Test for CLOCK_REALTIME */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include "posixtest.h"

/* pthread_rwlock_timedrdlock_3_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

#define TIMEOUT 3

static pthread_rwlock_t pthread_rwlock_timedrdlock_3_1_rwlock;
static int pthread_rwlock_timedrdlock_3_1_thread_state; 
static struct timeval pthread_rwlock_timedrdlock_3_1_currsec1, pthread_rwlock_timedrdlock_3_1_currsec2;

static void* pthread_rwlock_timedrdlock_3_1_fn_rd(void *arg)
{ 
	
	pthread_rwlock_timedrdlock_3_1_thread_state = ENTERED_THREAD;
	struct timespec timeout, ts;
	int rc;
#ifdef CLOCK_REALTIME
	printf("Test CLOCK_REALTIME\n");
	clock_gettime(CLOCK_REALTIME, &ts);
	pthread_rwlock_timedrdlock_3_1_currsec1.tv_sec = ts.tv_sec;
	pthread_rwlock_timedrdlock_3_1_currsec1.tv_usec = ts.tv_nsec / 1000;
#else
	gettimeofday(&pthread_rwlock_timedrdlock_3_1_currsec1, NULL);
#endif
	/* Absolute time, not relative. */
	timeout.tv_sec = pthread_rwlock_timedrdlock_3_1_currsec1.tv_sec + TIMEOUT;
	timeout.tv_nsec = pthread_rwlock_timedrdlock_3_1_currsec1.tv_usec * 1000;	
	
	printf("thread: attempt timed read lock, %d secs\n", TIMEOUT);	
	rc = pthread_rwlock_timedrdlock(&pthread_rwlock_timedrdlock_3_1_rwlock, &timeout);
	if(rc  == ETIMEDOUT)
		printf("thread: timer expired\n");
	else if(rc == 0)
	{
		printf("thread: acquired read lock\n");
		printf("thread: unlock read lock\n");
		if(pthread_rwlock_unlock(&pthread_rwlock_timedrdlock_3_1_rwlock) != 0)
		{
			return PTS_UNRESOLVED ;
		}
	}
	else
	{
		printf("Error: thread: in pthread_rwlock_timedrdlock(), return code:%d\n", rc);
		return PTS_UNRESOLVED ;
	}
	
	/* Get time after the pthread_rwlock_timedrdlock() call. */
#ifdef CLOCK_REALTIME
	clock_gettime(CLOCK_REALTIME, &ts);
	pthread_rwlock_timedrdlock_3_1_currsec2.tv_sec = ts.tv_sec;
	pthread_rwlock_timedrdlock_3_1_currsec2.tv_usec = ts.tv_nsec / 1000;
#else
	gettimeofday(&pthread_rwlock_timedrdlock_3_1_currsec2, NULL);
#endif
	pthread_rwlock_timedrdlock_3_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int pthread_rwlock_timedrdlock_3_1()
{
	int cnt = 0;
	pthread_t rd_thread1, rd_thread2;
	
	if(pthread_rwlock_init(&pthread_rwlock_timedrdlock_3_1_rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");
	if(pthread_rwlock_rdlock(&pthread_rwlock_timedrdlock_3_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired read lock\n");
	
	pthread_rwlock_timedrdlock_3_1_thread_state = NOT_CREATED_THREAD;
	
	printf("main: create rd_thread1\n");
	if(pthread_create(&rd_thread1, NULL, pthread_rwlock_timedrdlock_3_1_fn_rd, NULL) != 0)
	{
		printf("main: Error when creating rd_thread1\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after 5 seconds,
	   we regard it as blocked */

	/* we expect the thread not to block */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedrdlock_3_1_thread_state !=EXITING_THREAD && cnt++ < 5); 
	
	if(pthread_rwlock_timedrdlock_3_1_thread_state == ENTERED_THREAD)
	{
		/* the child thread started but blocked */
		printf("Test FAILED: rd_thread1 blocked on pthread_rwlock_timedrdlock()\n");
		return PTS_FAIL ;
	}
	else if(pthread_rwlock_timedrdlock_3_1_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state %d\n", pthread_rwlock_timedrdlock_3_1_thread_state);	
		return PTS_UNRESOLVED ;
	}
		
	if(pthread_join(rd_thread1, NULL) != 0)
	{
		printf("main: Error when join rd_thread1\n");
		return PTS_UNRESOLVED ;
	}

	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_timedrdlock_3_1_rwlock) != 0)
	{
		printf("main: Error when release read lock\n");
		return PTS_UNRESOLVED;	
	}
	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_timedrdlock_3_1_rwlock) != 0)
	{
		printf("main: Failed to get write lock\n");
		return PTS_UNRESOLVED;	
	}
	printf("main: acquired write lock\n");

	pthread_rwlock_timedrdlock_3_1_thread_state = NOT_CREATED_THREAD;
	printf("main: create rd_thread2\n");
	if(pthread_create(&rd_thread2, NULL, pthread_rwlock_timedrdlock_3_1_fn_rd, NULL) != 0)
	{
		printf("main: Failed to create rd_thread2\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* we expect rd_thread2 to block and timeout. */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_timedrdlock_3_1_thread_state !=EXITING_THREAD && cnt++ < 5); 
	
	if(pthread_rwlock_timedrdlock_3_1_thread_state == EXITING_THREAD)
	{
		/* the child thread does not block, check the time interval */
		struct timeval time_diff;
		time_diff.tv_sec = pthread_rwlock_timedrdlock_3_1_currsec2.tv_sec - pthread_rwlock_timedrdlock_3_1_currsec1.tv_sec;
		time_diff.tv_usec = pthread_rwlock_timedrdlock_3_1_currsec2.tv_usec - pthread_rwlock_timedrdlock_3_1_currsec1.tv_usec;
		if (time_diff.tv_usec < 0)
		{
			--time_diff.tv_sec;
			time_diff.tv_usec += 1000000;
		}
		if(time_diff.tv_sec < TIMEOUT)
		{
#if defined(_SIM_)
			/* qemu时钟差异较大，gap在10ms内测试通过 */
			if ((1000000 - time_diff.tv_usec)/1000 < 10) {
				printf("Test PASS: the timer expired and thread terminated, "
					"start time %ld.%06ld, end time %ld.%06ld\n", 
					(long) pthread_rwlock_timedrdlock_3_1_currsec1.tv_sec, (long) pthread_rwlock_timedrdlock_3_1_currsec1.tv_usec, 
					(long) pthread_rwlock_timedrdlock_3_1_currsec2.tv_sec, (long) pthread_rwlock_timedrdlock_3_1_currsec2.tv_usec);
			} else {
#endif
			printf("Test FAILED: the timer expired and thread terminated, "
				"but the timeout is not correct: "
				"start time %ld.%06ld, end time %ld.%06ld\n", 
				(long) pthread_rwlock_timedrdlock_3_1_currsec1.tv_sec, (long) pthread_rwlock_timedrdlock_3_1_currsec1.tv_usec, 
				(long) pthread_rwlock_timedrdlock_3_1_currsec2.tv_sec, (long) pthread_rwlock_timedrdlock_3_1_currsec2.tv_usec);
			return PTS_FAIL ;
#if defined(_SIM_)
			}
#endif
		} else
			printf("thread: read lock correctly timed out\n");
	}
	else if(pthread_rwlock_timedrdlock_3_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: read block was not terminated even when the timer expired\n");
		return PTS_FAIL ;
	}
	else
	{
		printf("Unexpected thread state %d\n", pthread_rwlock_timedrdlock_3_1_thread_state);
		return PTS_UNRESOLVED;
	}

	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_timedrdlock_3_1_rwlock) != 0)
	{
		printf("main: Failed to release write lock\n");
		return PTS_UNRESOLVED ;
	}

	if(pthread_rwlock_destroy(&pthread_rwlock_timedrdlock_3_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()\n");
		return PTS_UNRESOLVED ;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
