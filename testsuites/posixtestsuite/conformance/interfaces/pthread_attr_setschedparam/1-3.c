/*
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.

 * Test that pthread_create(pthread_attr)
 *   shall create a thread with the pthread_attr settings passed to it

 * Steps:
 * 1. Create a pthread_attr structure and set policy and priority in it
 * 2. Create a thread using this attr
 * 3. In the thread, check these settings.

 */

#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "3-1"
#define AREA "scheduler"
#define ERROR_PREFIX "unexpected error: " AREA " " TEST ": "

#define PRIORITY 20
#define POLICY SCHED_FIFO

/* the thread uses this to indicate to main or success */
int pthread_attr_setschedparam_1_3_policy_correct = -1;	
/* the thread uses this to indicate to main or success */
int pthread_attr_setschedparam_1_3_priority_correct = -1;	


/* Thread function which checks the scheduler settings for itself */
void *pthread_attr_setschedparam_1_3_thread(void *tmp)
{
	struct sched_param   param;
	int                  policy;
	if(pthread_getschedparam(pthread_self(), &policy, &param) != 0) {
		printf(ERROR_PREFIX "pthread_getschedparam\n");
		return PTS_UNRESOLVED;
	}
	if(policy == POLICY) {
		pthread_attr_setschedparam_1_3_policy_correct = 1;
	}
	if(param.sched_priority == PRIORITY) {
		pthread_attr_setschedparam_1_3_priority_correct = 1;
	}
	return NULL;
}

int pthread_attr_setschedparam_1_3()
{
	pthread_t            thread_id;
	pthread_attr_t       attr;
	struct sched_param   param;
	int                  rc = 0;

	/* initialze the attribute and set policy and priority in it*/
	rc = pthread_attr_init(&attr);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		return PTS_UNRESOLVED;
	}
	rc = pthread_attr_setschedpolicy(&attr, POLICY);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		return PTS_UNRESOLVED;
	}
	param.sched_priority = PRIORITY;
	rc = pthread_attr_setschedparam(&attr, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam\n");
		return PTS_UNRESOLVED;
	}
	
	rc = pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setinheritsched\n");
		return PTS_UNRESOLVED;
	}

	/* Create the thread with the attr */
	rc = pthread_create(&thread_id, &attr, pthread_attr_setschedparam_1_3_thread, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_create\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_join(thread_id, NULL);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_join\n");
		return PTS_UNRESOLVED;
	}

	/* test the result */
	if(pthread_attr_setschedparam_1_3_priority_correct != 1) {
		printf("Test FAILED. Priority set incorrectly\n");
		return PTS_FAIL;
	}
	if(pthread_attr_setschedparam_1_3_policy_correct != 1) {
		printf("Test FAILED. Policy set incorrectly\n");
		return PTS_FAIL;
	}

	printf("Test PASS\n");
	return PTS_PASS;
}
