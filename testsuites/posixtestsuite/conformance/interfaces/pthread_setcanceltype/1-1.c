/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  rolla.n.selbak REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_setcanceltype
 * Atomically sets the cancelability type to 'type' and returns the old
 * cancelability type in the location referenced by 'oldtype'.
 * 'state' can either be PTHREAD_CANCEL_DEFERRED, or PTHREAD_CANCEL_ASYNCHRONOUS.
 *
 * Test when a thread is PTHREAD_CANCEL_ASYNCHRONOUS
 *  
 * STEPS:
 * 1. Setup a pthread_setcanceltype_1_1_mutex and lock it in main()
 * 2. Create a thread.
 * 3. In the thread function, set the type to PTHREAD_CANCEL_ASYNCHRONOUS
 * 4. Setup a cleanup handler for the thread.
 * 5. Make the thread block on the locked pthread_setcanceltype_1_1_mutex
 * 6. Send out a thread cancel request to the new thread
 * 7. If the cancel request was honored immediately and correctly, the
 *    cleanup handler would have been executed, and the test will pass. 
 * 8. If not, main will wait for 10 seconds before it  unlocks the pthread_setcanceltype_1_1_mutex, and the thread
 *    will exit, failing the test.     
 */

#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

# define INTHREAD 0 	/* Control going to or is already for Thread */
# define INMAIN 1	/* Control going to or is already for Main */
# define TIMEOUT 10	/* Time out time in seconds */

volatile int pthread_setcanceltype_1_1_sem1;		/* Manual semaphore */
volatile int pthread_setcanceltype_1_1_cleanup_flag;	/* Flag to indicate the thread's cleanup handler was called */
pthread_mutex_t	pthread_setcanceltype_1_1_mutex = PTHREAD_MUTEX_INITIALIZER;	/* Mutex */


/* Cleanup function that the thread executes when it is canceled.  So if
 * pthread_setcanceltype_1_1_cleanup_flag is 1, it means that the thread was canceled. */
void pthread_setcanceltype_1_1_a_cleanup_func(void *unused)
{
	pthread_setcanceltype_1_1_cleanup_flag=1;
	return;
}

/* Function that the thread executes upon its creation */
void *pthread_setcanceltype_1_1_a_thread_func()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);
	
	pthread_cleanup_push(pthread_setcanceltype_1_1_a_cleanup_func,NULL);
	
	/* Indicate to main() that the thread has been created. */	
	pthread_setcanceltype_1_1_sem1=INMAIN;

	/* Lock the pthread_setcanceltype_1_1_mutex. It should have already been locked in main, so the thread
	 * should block. */
	if(pthread_mutex_lock(&pthread_setcanceltype_1_1_mutex) != 0)
        {
		perror("Error in pthread_mutex_lock()\n");
		pthread_exit((void*)PTS_UNRESOLVED);
		return (void*)PTS_UNRESOLVED;
	}

	/* Shouldn't get here if the cancel request was honored immediately
	 * like it should have been. */
	pthread_setcanceltype_1_1_cleanup_flag=-1;
	pthread_cleanup_pop(0);
	pthread_exit(0);
	return NULL;
}

int pthread_setcanceltype_1_1()
{
	pthread_t new_th;
	int i=0;

	/* Initializing values */
	pthread_setcanceltype_1_1_sem1=INTHREAD;
	pthread_setcanceltype_1_1_cleanup_flag=0;
	
	/* Lock the pthread_setcanceltype_1_1_mutex */
	if(pthread_mutex_lock(&pthread_setcanceltype_1_1_mutex) != 0)
	{
		perror("Error in pthread_mutex_lock()\n");
		return PTS_UNRESOLVED;
	}
	
	/* Create a new thread. */
	if(pthread_create(&new_th, NULL, pthread_setcanceltype_1_1_a_thread_func, NULL) != 0)
	{	
		perror("Error creating thread\n");
		return PTS_UNRESOLVED;
	}
	
	/* Make sure thread is created before we cancel it. (wait for 
	 * pthread_setcanceltype_1_1_a_thread_func() to set pthread_setcanceltype_1_1_sem1=INMAIN.) */
	while(pthread_setcanceltype_1_1_sem1==INTHREAD)
		sleep(1);

	/* Send cancel request to the thread.  */
	if(pthread_cancel(new_th) != 0) 
	{
		perror("Test FAILED: Error in pthread_cancel()\n");
		return PTS_UNRESOLVED;
	}
	/* Wait for the thread to either cancel immediately (as it should do) and call it's
	 * cleanup handler, or for TIMEOUT(10) seconds if the cancel request was not honored
	 * immediately. */ 
	while((pthread_setcanceltype_1_1_cleanup_flag == 0) && (i != TIMEOUT))
	{
		sleep(1);
		i++;
	}

	/* Unlock the pthread_setcanceltype_1_1_mutex */
	pthread_mutex_unlock(&pthread_setcanceltype_1_1_mutex);

	/* This means that the cleanup function wasn't called, so the cancel
	 * request was not honord immediately like it should have been. */
	if(pthread_setcanceltype_1_1_cleanup_flag <= 0)
	{
		printf("Test FAILED: Cancel request timed out\n");
		return PTS_FAIL;
	}	
	
	printf("Test PASSED\n");
	return PTS_PASS;	
}


