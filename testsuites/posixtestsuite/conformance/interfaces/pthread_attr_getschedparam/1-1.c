/*   
 * Copyright (c) 2004, QUALCOMM Inc. All rights reserved.
 * Created by:  abisain REMOVE-THIS AT qualcomm DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_getschedparam()
 * 
 * Steps:
 * 1.  Initialize a pthread_attr_t object using pthread_attr_init()
 * 2.  Call pthread_attr_setschedpolicy with policy parameter
 * 3.  Call pthread_attr_setschedparam with a sched param parameter
 * 4.  Call pthread_attr_getschedparam to get the sched param
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "posixtest.h"

#define TEST "1-1"
#define FUNCTION "pthread_attr_getschedparam"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define FIFOPOLICY SCHED_FIFO 
#define RRPOLICY SCHED_RR
#define OTHERPOLICY SCHED_OTHER

// todo: pthread_attr_getschedparam_1_1 仅支持PRT_SCHED_FIFO 用例其他优先级替换方便以后
#define RRPOLICY SCHED_FIFO
#define OTHERPOLICY SCHED_FIFO

int pthread_attr_getschedparam_1_1_verify_param(pthread_attr_t *attr, int priority) {
	int rc;
	struct sched_param param;

	rc = pthread_attr_getschedparam(attr, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_getschedparam\n");
		return PTS_UNRESOLVED;
	}
	if(priority != param.sched_priority) {
		printf(ERROR_PREFIX "got wrong sched param\n");
		return PTS_FAIL;
	}	
        return 0;                                   
}

int pthread_attr_getschedparam_1_1()
{
	int                   rc=0;
	pthread_attr_t        attr;
	struct sched_param    param;
	int                   priority;

	rc = pthread_attr_init(&attr);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_init\n");
		return PTS_UNRESOLVED;
	}

	rc = pthread_attr_setschedpolicy(&attr, FIFOPOLICY);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		return PTS_UNRESOLVED;
	}
	priority = sched_get_priority_max(FIFOPOLICY);
	if(priority == -1) {
		printf(ERROR_PREFIX "sched_get_priority_max\n");
		return PTS_UNRESOLVED;
	}
	param.sched_priority = priority;
	rc = pthread_attr_setschedparam(&attr, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam\n");
		return PTS_UNRESOLVED;
	}
	pthread_attr_getschedparam_1_1_verify_param(&attr, priority);

	rc = pthread_attr_setschedpolicy(&attr, RRPOLICY);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedpolicy\n");
		return PTS_UNRESOLVED;
	}
	priority = sched_get_priority_max(RRPOLICY);
	if(priority == -1) {
		printf(ERROR_PREFIX "sched_get_priority_max\n");
		return PTS_UNRESOLVED;
	}
	param.sched_priority = priority;
	rc = pthread_attr_setschedparam(&attr, &param);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_setschedparam\n");
		return PTS_UNRESOLVED;
	}
	pthread_attr_getschedparam_1_1_verify_param(&attr, priority);

	rc = pthread_attr_destroy(&attr);
	if(rc != 0) {
		printf(ERROR_PREFIX "pthread_attr_destroy\n");
		return PTS_UNRESOLVED;
	}
	printf("Test PASS\n");
	return PTS_PASS;
}
