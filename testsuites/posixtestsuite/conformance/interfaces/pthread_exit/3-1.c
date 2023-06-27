/*   
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_exit()
 *  
 * Any destructors for thread_specific data will be called when 
 * pthread_exit is called
 * 
 * Steps:
 * 1. Create a new thread.
 * 2. Create thread specific data, with a pthread_exit_3_1_destructor in the thread
 * 3. Call pthread_exit in the thread.
 * 4. Make sure that the pthread_exit_3_1_destructor was called
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "3-1"
#define FUNCTION "pthread_exit"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

/* Flag to indicate that the pthread_exit_3_1_destructor was called */
int pthread_exit_3_1_cleanup_flag = 0;

void pthread_exit_3_1_destructor(void *tmp)
{
	pthread_exit_3_1_cleanup_flag = 1;
}

/* Thread's function. */
void *pthread_exit_3_1_a_thread_func(void *tmp)
{
	pthread_key_t    key;
	int              value = 1;
	int              rc = 0;

	rc = pthread_key_create(&key, pthread_exit_3_1_destructor);
	if(rc != 0 ) {	
		printf(ERROR_PREFIX "pthread_key_create\n");
		return PTS_UNRESOLVED ;
	}

	rc = pthread_setspecific(key, &value);
	if(rc != 0 ) {	
		printf(ERROR_PREFIX "pthread_setspecific\n");
		return PTS_UNRESOLVED ;
	}
	
	pthread_exit(0);
	return NULL;
}

int pthread_exit_3_1()
{
	pthread_t new_th;
	int       rc = 0;
	
	/* Create a new thread. */
	rc = pthread_create(&new_th, NULL, pthread_exit_3_1_a_thread_func, NULL);
	if(rc != 0) {	
		printf(ERROR_PREFIX "pthread_create\n");
		return PTS_UNRESOLVED;
	}
	
	/* Wait for thread to return */
	rc = pthread_join(new_th, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_join\n");
		return PTS_UNRESOLVED;
	}

	if(pthread_exit_3_1_cleanup_flag != 1 ) {
		printf("Test FAIL: Destructor was not called.\n");
		return PTS_FAIL;
	}

	printf("Test PASS\n");
	return PTS_PASS;
	
}
