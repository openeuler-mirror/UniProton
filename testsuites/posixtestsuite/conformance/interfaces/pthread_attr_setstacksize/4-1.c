/*   
 * Copyright (c) 2004, Intel Corporation. All rights reserved.
 * Created by:  crystal.xiong REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test pthread_attr_setstacksize()
 * 
 * Steps:
 * 1.  Initialize pthread_attr_t object (attr) 
 * 2.  set the stacksize less tha PTHREAD_STACK_MIN 
 */

#include <pthread.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/param.h>
#include <errno.h>
#include <unistd.h>
#include "posixtest.h"

#define TEST "4-1"
#define FUNCTION "pthread_attr_setstacksize"
#define ERROR_PREFIX "unexpected error: " FUNCTION " " TEST ": "

#define STACKSIZE (PTHREAD_STACK_MIN > sysconf(_SC_PAGE_SIZE)) ? (PTHREAD_STACK_MIN - sysconf(_SC_PAGE_SIZE)) : (PTHREAD_STACK_MIN/2)

void *pthread_attr_setstacksize_4_1_thread_func()
{
	pthread_exit(0);
	return NULL;
}
int pthread_attr_setstacksize_4_1()
{
	pthread_attr_t attr;
	void *saddr;
	size_t stack_size;
	int rc;

	/* Initialize attr */
	rc = pthread_attr_init(&attr);
	if( rc != 0) {
		perror(ERROR_PREFIX "pthread_attr_init");
		return PTS_UNRESOLVED ;
	}
	
	stack_size = STACKSIZE;

	if (posix_memalign (&saddr, sysconf(_SC_PAGE_SIZE), 
            stack_size) != 0)
    	{
      		perror (ERROR_PREFIX "out of memory while "
                        "allocating the stack memory");
      		return PTS_UNRESOLVED ;
    	}

	rc = pthread_attr_setstacksize(&attr, stack_size);
        if (rc != EINVAL ) {
                perror(ERROR_PREFIX "Got the wrong return value");
                return PTS_FAIL ;
        }

	rc = pthread_attr_destroy(&attr);
	if(rc != 0)
        {
                perror(ERROR_PREFIX "pthread_attr_destroy");
		return PTS_UNRESOLVED ;
        }
	
	printf("Test PASSED\n");
	return PTS_PASS;
}


