/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_rdlock(pthread_rwlock_t *pthread_rwlock_rdlock_1_1_rwlock)
 *
 * pthread_rwlock_rdlock( ) function shall apply a read lock to the 
 * read-write lock referenced by pthread_rwlock_rdlock_1_1_rwlock. The calling thread acquires 
 * the read lock if a writer does not hold the lock and there are
 * no writers blocked on the lock.
 *
 * NOTE: This case will not test whether a thread can get the read lock if
 * there are writers blocked on the lock -- it is implementation dependant.
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_rdlock_1_1_rwlock' with pthread_rwlock_init()
 * 2.  Main thread read lock 'pthread_rwlock_rdlock_1_1_rwlock'
 * 3.  Create a child thread. The thread read lock 'pthread_rwlock_rdlock_1_1_rwlock', should not block. Then unlock it.
 * 4.  Main thread unlock 'pthread_rwlock_rdlock_1_1_rwlock'
 * 5.  Main thread write lock 'pthread_rwlock_rdlock_1_1_rwlock'
 * 6.  Create child thread to read lock 'pthread_rwlock_rdlock_1_1_rwlock', should block
 * 7.  Main thread unlock 'pthread_rwlock_rdlock_1_1_rwlock'
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_rwlock_t pthread_rwlock_rdlock_1_1_rwlock;
static int pthread_rwlock_rdlock_1_1_thread_state; 

/* pthread_rwlock_rdlock_1_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_rdlock_1_1_fn_rd(void *arg)
{ 
	int rc = 0;
	pthread_rwlock_rdlock_1_1_thread_state = ENTERED_THREAD;
	printf("rd_thread: attempt read lock\n");
	rc = pthread_rwlock_rdlock(&pthread_rwlock_rdlock_1_1_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: rd_thread failed to get read lock, Error code:%d\n"
			, rc);
		return PTS_FAIL ;
	} else
		printf("rd_thread: acquired read lock\n");

	sleep(1);

	printf("rd_thread: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_rdlock_1_1_rwlock) != 0)
	{
		printf("rd_thread: Error at pthread_rwlock_unlock()\n");
		return PTS_UNRESOLVED ;
	}
	pthread_rwlock_rdlock_1_1_thread_state = EXITING_THREAD;
	pthread_exit(0);
	return NULL;
}
 
int pthread_rwlock_rdlock_1_1()
{
	int cnt = 0;
	pthread_t rd_thread1, rd_thread2;
	
	if(pthread_rwlock_init(&pthread_rwlock_rdlock_1_1_rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");

	/* We have no lock, this read lock should succeed */	
	if(pthread_rwlock_rdlock(&pthread_rwlock_rdlock_1_1_rwlock) != 0)
	{
		printf("Test FAILED: main cannot get read lock when no one owns the lock\n");
		return PTS_FAIL;
	} else
		printf("main: acquired read lock\n");
	
	pthread_rwlock_rdlock_1_1_thread_state = NOT_CREATED_THREAD;
	
	printf("main: create rd_thread1\n");
	if(pthread_create(&rd_thread1, NULL, pthread_rwlock_rdlock_1_1_fn_rd, NULL) != 0)
	{
		printf("main: Error at 1st pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_rdlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_rdlock_1_1_thread_state == ENTERED_THREAD)
	{
		/* the child thread started but blocked */
		printf("Test FAILED: thread blocked on read lock, when a reader owns the lock\n");
		return PTS_FAIL ;
	}
	else if(pthread_rwlock_rdlock_1_1_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state\n");
		return PTS_UNRESOLVED ;
	}

	printf("main: unlock read lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_rdlock_1_1_rwlock) != 0)
	{
		printf("main: failed to release read lock\n");
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_join(rd_thread1, NULL) != 0)
	{
		printf("main: Error at 1st pthread_join()\n");
		return PTS_UNRESOLVED ;
	}
	

	/* Passed first part of the test, now for the second part. */

	
	printf("main: attempt write lock\n");
	if(pthread_rwlock_wrlock(&pthread_rwlock_rdlock_1_1_rwlock) != 0)
	{
		printf("main: failed to get write lock\n");
		return PTS_UNRESOLVED;	
	} else
		printf("main: acquired write lock\n");

	pthread_rwlock_rdlock_1_1_thread_state = NOT_CREATED_THREAD;
	printf("main: create rd_thread2\n");
	if(pthread_create(&rd_thread2, NULL, pthread_rwlock_rdlock_1_1_fn_rd, NULL) != 0)
	{
		printf("main: failed at 2nd pthread_create()\n");
		return PTS_UNRESOLVED;
	}
	
	cnt = 0;
	/* Expect the child thread block on read lock */
	do{
		sleep(1);
	}while (pthread_rwlock_rdlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_rdlock_1_1_thread_state == EXITING_THREAD)
	{
		printf("Test FAILED: thread did not block on read lock when a writer holds the lock\n");
		return PTS_FAIL;
	}
	else if(pthread_rwlock_rdlock_1_1_thread_state != ENTERED_THREAD)
	{
		printf("main: Unexpected thread state\n");
		return PTS_UNRESOLVED;
	}

	printf("main: unlock write lock\n");
	if(pthread_rwlock_unlock(&pthread_rwlock_rdlock_1_1_rwlock) != 0)
	{
		printf("main: failed to release write lock\n");
		return PTS_UNRESOLVED;
	}

	/* We expected the child get the read lock and exit */
	cnt = 1;	
	do{
		sleep(1);
	}while (pthread_rwlock_rdlock_1_1_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_rdlock_1_1_thread_state == ENTERED_THREAD)
	{
		printf("Test FAILED: thread blocked on read lock after the writer released the lock\n");
		return PTS_FAIL;
	}
	else if(pthread_rwlock_rdlock_1_1_thread_state != EXITING_THREAD)
	{
		printf("main: Unexpected thread state\n");
		return PTS_UNRESOLVED;
	}

	if(pthread_rwlock_destroy(&pthread_rwlock_rdlock_1_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlockattr_destroy()");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
