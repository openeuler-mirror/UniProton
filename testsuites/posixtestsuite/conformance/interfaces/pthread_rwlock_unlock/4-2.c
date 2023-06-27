/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_rwlock_unlock(pthread_rwlock_t *pthread_rwlock_unlock_4_3_rwlock)
 *
 *	It 'may' fail if:
 *	[EINVAL]  pthread_rwlock_unlock_4_3_rwlock doesn't refer to an intialized read-write lock
 *	[EPERM]  the current thread doesn't hold the lock on the pthread_rwlock_unlock_4_3_rwlock
 *
 *	Testing EPERM in this test.
 *
 *
 * Steps:
 * 1.  Initialize a pthread_rwlock_t object 'pthread_rwlock_unlock_4_3_rwlock' with pthread_rwlock_init()
 * 2.  Main thread read lock 'pthread_rwlock_unlock_4_3_rwlock'
 * 3.  Create a child thread, the thread should try to unlock the 'pthread_rwlock_unlock_4_3_rwlock'
 * 4. The test will pass even if it returns 0, but with a note stating that the standard
 *     states it 'may' fail.
 */

#define _XOPEN_SOURCE 600
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "posixtest.h"

static pthread_rwlock_t pthread_rwlock_unlock_4_3_rwlock;
static int pthread_rwlock_unlock_4_3_rc, pthread_rwlock_unlock_4_3_thread_state; 

/* pthread_rwlock_unlock_4_3_thread_state indicates child thread state: 
	1: not in child thread yet; 
	2: just enter child thread ;
	3: just before child thread exit;
*/

#define NOT_CREATED_THREAD 1
#define ENTERED_THREAD 2
#define EXITING_THREAD 3

static void* pthread_rwlock_unlock_4_3_fn_unlk(void *arg)
{ 
	pthread_rwlock_unlock_4_3_thread_state = ENTERED_THREAD;
	printf("un_thread: unlock read lock\n");
	pthread_rwlock_unlock_4_3_rc = pthread_rwlock_unlock(&pthread_rwlock_unlock_4_3_rwlock);
	pthread_rwlock_unlock_4_3_thread_state = EXITING_THREAD;
	return NULL;
}
 
int pthread_rwlock_unlock_4_2()
{
	int cnt = 0;
	int pthread_rwlock_unlock_4_3_rc = 0;

	pthread_t un_thread;
	
	if(pthread_rwlock_init(&pthread_rwlock_unlock_4_3_rwlock, NULL) != 0)
	{
		printf("main: Error at pthread_rwlock_init()\n");
		return PTS_UNRESOLVED;
	}

	printf("main: attempt read lock\n");

	if(pthread_rwlock_rdlock(&pthread_rwlock_unlock_4_3_rwlock) != 0)
	{
		printf("main: Error at pthread_rwlock_rdlock()\n");
		return PTS_UNRESOLVED;
	}
	printf("main: acquired read lock\n");

	pthread_rwlock_unlock_4_3_thread_state = NOT_CREATED_THREAD;
	
	printf("main: create un_thread\n");
	if(pthread_create(&un_thread, NULL, pthread_rwlock_unlock_4_3_fn_unlk, NULL) != 0)
	{
		printf("main: Error at pthread_create()\n");
		return PTS_UNRESOLVED;
	}

	// 增加调度点
	sched_yield();
	
	/* Wait for child to exit */
	cnt = 0;
	do{
		sleep(1);
	}while (pthread_rwlock_unlock_4_3_thread_state !=EXITING_THREAD && cnt++ < 3); 
	
	if(pthread_rwlock_unlock_4_3_thread_state != EXITING_THREAD)
	{
		printf("Unexpected thread state %d\n", pthread_rwlock_unlock_4_3_thread_state);
		return PTS_UNRESOLVED ;
	}
	
	if(pthread_join(un_thread, NULL) != 0)
	{
		printf("Error at pthread_join()\n");
		return PTS_UNRESOLVED ;
	}
	
	/* Cleaning up */
	pthread_rwlock_unlock(&pthread_rwlock_unlock_4_3_rwlock);
	if(pthread_rwlock_destroy(&pthread_rwlock_unlock_4_3_rwlock) != 0)
	{
		printf("error at pthread_rwlock_destroy()\n");
		return PTS_UNRESOLVED;
	}	

	/* Test the return code of un_thread when it attempt to unlock the pthread_rwlock_unlock_4_3_rwlock it didn't
	 * own in the first place. */
	
	if(pthread_rwlock_unlock_4_3_rc != 0)
	{
		if(pthread_rwlock_unlock_4_3_rc == EPERM)
		{
			printf("Test PASSED\n");
			return PTS_PASS;
		}

		printf("Test FAILED: Incorrect error code, expected 0 or EPERM, got %d\n", pthread_rwlock_unlock_4_3_rc);
		return PTS_FAIL;
	}
	
	printf("Test PASSED: Note*: Returned 0 instead of EPERM, but standard specified _may_ fail.\n");
	return PTS_PASS;
}
