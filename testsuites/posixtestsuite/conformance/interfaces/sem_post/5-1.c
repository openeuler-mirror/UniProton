/*
 * Copyright (c) 2003, Intel Corporation. All rights reserved.
 * Created by:  majid.awad REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 */

/*
 * sem_post() can be entered into again if interrupted by a signal
 * --> This is tough to do with 100% guarantees.  This test does a
 * best attempt.
 *
 * 1) Set up a signal catching function for SIGALRM.
 * 2) Set up a semaphore
 * 3) Call sleep(1) and then alarm(1)
 * 4) Call sempost() immediately after (thinking that alarm(1) will interrupt
 * 5) If sempost() succeeds (verified if val is incremented by 1),
 *    test passes.  Otherwise, failure.
 */


#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
// #include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include "posixtest.h"


#define TEST "5-1"
#define FUNCTION "sem_post"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define SEMINITVAL 0 //initial value of semaphore = 0

sem_t *sem_post_5_1_gsemp;

void sem_post_5_1_sighdl(int sig)
{
	return;
}

int sem_post_5_1() {
	char semname[50];
	int val;

	sprintf(semname, "/" FUNCTION "_" TEST "_%d", getpid());

	sem_post_5_1_gsemp = sem_open(semname, O_CREAT, 0777, SEMINITVAL);
	if( sem_post_5_1_gsemp == SEM_FAILED || sem_post_5_1_gsemp == NULL ) {
		perror(ERROR_PREFIX "sem_open");
		return PTS_UNRESOLVED;
	}
	signal(SIGALRM, sem_post_5_1_sighdl);
	sleep(1);
        alarm(1);

	if( sem_post(sem_post_5_1_gsemp) == -1 ) {
		perror(ERROR_PREFIX "sem_post");
		return PTS_UNRESOLVED ; 
	}

	/* Checking if the value of the Semaphore incremented by one */
	if( sem_getvalue(sem_post_5_1_gsemp, &val) == -1 ) {
		perror(ERROR_PREFIX "sem_getvalue");
		return PTS_UNRESOLVED;
	}
	if (val != SEMINITVAL+1) {
#ifdef DEBUG
		printf("semaphore value was not incremented\n");
#endif
		printf("TEST FAILED\n");
		return PTS_FAIL;
	}

#ifdef DEBUG
	printf("semaphore value was %d\n", val);
#endif

	printf("TEST PASSED\n");
	sem_close(sem_post_5_1_gsemp);
	sem_unlink(semname);
	return PTS_PASS;
}

