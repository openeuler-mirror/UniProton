/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_unlock(pthread_rwlock_t *pthread_rwlock_unlock_2_1_rwlock)
 *
 * pthread_rwlock_unlock( ) function shall release a lock held on the 
 * read-write lock object referenced by pthread_rwlock_unlock_2_1_rwlock
 * If this function is called to release a write lock for this read-write 
 * lock object, the read-write lock object shall be put in the unlocked state.
 *
 * In the case: if a lock in an unlocked state, it can be acquired by a thread for write lock
 * successfully
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_unlock_2_1_rwlock' with pthread_rwlock_init()
 * 2.  Main thread write lock 'pthread_rwlock_unlock_2_1_rwlock'
 * 3.  Create a child thread, the thread write lock 'pthread_rwlock_unlock_2_1_rwlock', should block
 * 4.  Main thread unlock the write lock, the 'pthread_rwlock_unlock_2_1_rwlock' is in unlocked state
 * 5.  Child thread should get the lock for writing.
 */
#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

static pthread_rwlock_t pthread_rwlock_unlock_2_1_rwlock;
static int pthread_rwlock_unlock_2_1_thread_state; 

/* pthread_rwlock_unlock_2_1_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_unlock_2_1_fn_wr(void *arg)
{ 
	int rc = 0;

	pthread_rwlock_unlock_2_1_thread_state = ENTERED_THREAD;
	printf("thread: attempt write block\n");
	/* This should block */
	if(pthread_rwlock_wrlock(&pthread_rwlock_unlock_2_1_rwlock) != 0)
	{
		printf("thread: cannot get write lock\n");
		return PTS_UNRESOLVED ;
	}
	printf("thread: acquired write lock\n");
	printf("thread: unlock write lock\n");
	rc = pthread_rwlock_unlock(&pthread_rwlock_unlock_2_1_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: thread failed to release write lock, Error Code=%d\n", rc);
		return PTS_FAIL ;
	}
	pthread_rwlock_unlock_2_1_thread_state = EXITING_THREAD;
	return NULL;
}
 
int pthread_rwlock_unlock_2_1()
{
	int cnt = 0;
	int rc = 0;

	pthread_t wr_thread;
	
	if(pthread_rwlock_init(&pthread_rwlock_unlock_2_1_rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt write lock\n");
	/* This write lock should succeed */	
	if(pthread_rwlock_wrlock(&pthread_rwlock_unlock_2_1_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_wrlock()\n");
		return PTS_UNRESOLVED;
	}
	
	pthread_rwlock_unlock_2_1_thread_state = NOT_CREATED_THREAD;
	printf("main: create thread\n");
	if(pthread_create(&wr_thread, NULL, pthread_rwlock_unlock_2_1_fn_wr, NULL) != 0)
	{
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* If the shared data is not altered by child after 3 seconds,
	   we regard it as blocked */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_unlock_2_1_thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_unlock_2_1_thread_state == EXITING_THREAD)
	{
		printf("Thread should block on write lock\n");
		return PTS_UNRESOLVED ;
	}
	else if(pthread_rwlock_unlock_2_1_thread_state != ENTERED_THREAD)
	{
		printf("Unexpected thread state: %d\n", pthread_rwlock_unlock_2_1_thread_state);
		return PTS_UNRESOLVED ;
	}

	/* pthread_rwlock_unlock_2_1_thread_state == ENTERED_THREAD, i.e. thread correctly blocks on write lock */
	printf("main: unlock write lock\n");
	rc = pthread_rwlock_unlock(&pthread_rwlock_unlock_2_1_rwlock);
	if(rc != 0)
	{
		printf("Test FAILED: Main cannot release write lock\n");
		return PTS_FAIL ;
	}

	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_unlock_2_1_thread_state != EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_unlock_2_1_thread_state != EXITING_THREAD)
	{
		printf("Test FAILED: thread did not get write lock even when the lock has no owner\n");
		return PTS_FAIL ;
	}

	if(pthread_join(wr_thread, NULL) != 0)
	{
		printf("Error at pthread_join()\n");
		return PTS_UNRESOLVED ;
	}

	if(pthread_rwlock_destroy(&pthread_rwlock_unlock_2_1_rwlock) != 0)
	{
		printf("Error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	printf("Test PASSED\n");
	return PTS_PASS;
}
