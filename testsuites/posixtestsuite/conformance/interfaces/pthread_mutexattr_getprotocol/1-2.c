/*   
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  bing.wei.liu REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this 
 * source tree.

 * Test that pthread_mutexattr_getprotocol()
 *
 * Gets the protocol attribute of a mutexattr object (which was prev. created
 * by the function pthread_mutexattr_init()).
 * 
 */

#include <pthread.h>
#include <stdio.h>
#include <sched.h>
#include <stdlib.h>
#include "posixtest.h"

// todo: PTHREAD_PRIO_PROTECT暂时不支持 
#define RUN_NUM 2

int pthread_mutexattr_getprotocol_1_2()
{
	
	pthread_mutexattr_t mta;
	int protocol, protcls[3],i;
	
	/* Initialize a mutex attributes object */
	if(pthread_mutexattr_init(&mta) != 0)
	{
		perror("Error at pthread_mutexattr_init()\n");
		return PTS_UNRESOLVED;
	}
	
	protcls[0]=PTHREAD_PRIO_NONE;
	protcls[1]=PTHREAD_PRIO_INHERIT;
	protcls[2]=PTHREAD_PRIO_PROTECT;
	
	for(i=0;i<RUN_NUM;i++)
	{
		/* Set the protocol to one of the 3 valid protocols. */
		if(pthread_mutexattr_setprotocol(&mta,protcls[i]))
		{
			printf("Error setting protocol to %d\n", protcls[i]);
			return PTS_UNRESOLVED;
		}

		/* Get the protocol mutex attr. */
		if(pthread_mutexattr_getprotocol(&mta, &protocol) != 0)
		{
			printf("Error obtaining the protocol attribute.\n");
			return PTS_UNRESOLVED;
		}

		/* Make sure that the protocol set is the protocl we get when calling
		 * pthread_mutexattr_getprocol() */
		if(protocol != protcls[i])
		{
			printf("Test FAILED: Set protocol %d, but instead got protocol %d.\n", protcls[i], protocol);
			return PTS_FAIL;
		}	
	}

	printf("Test PASSED\n");
	return PTS_PASS;
}
