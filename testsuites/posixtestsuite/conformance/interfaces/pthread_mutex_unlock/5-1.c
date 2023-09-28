/*
 * Copyright (c) 2004, Bull S.A..  All rights reserved.
 * Created by: Sebastien Decugis

 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 
 
 * This sample test aims to check the following assertion:
 * If the mutex type is PTHREAD_MUTEX_RECURSIVE,
 * and a thread attempts to unlock a mutex that it does not own,
 * an error is returned.

 * The steps are:
 *  -> Initialize and lock a recursive mutex
 *  -> create a child thread which tries to unlock this mutex. * 
 */

 /* 
  * - adam.li@intel.com 2004-05-20
  *   Add to PTS. Please refer to http://nptl.bullopensource.org/phpBB/ 
  *   for general information
  */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L
 
 /* We enable the following line to have mutex attributes defined */
#ifndef WITHOUT_XOPEN
 #define _XOPEN_SOURCE	600
 
/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <stdio.h>
 #include <stdarg.h>

 #include <errno.h> /* needed for EPERM test */
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "testfrmw.h"
 #include "testfrmw.c"
 /* This header is responsible for defining the following macros:
  * UNRESOLVED(ret, descr);  
  *    where descr is a description of the error and ret is an int (error code for example)
  * FAILED(descr);
  *    where descr is a short text saying why the test has failed.
  * PASSED();
  *    No parameter.
  * 
  * Both three macros shall terminate the calling process.
  * The testcase shall not terminate in any other maneer.
  * 
  * The other file defines the functions
  * void pthread_mutex_unlock_output_init()
  * void printf(char * string, ...)
  * 
  * Those may be used to printf information.
  */

/********************************************************************************************/
/********************************** Configuration ******************************************/
/********************************************************************************************/
#ifndef VERBOSE
#define VERBOSE 1
#endif

/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/

pthread_mutex_t pthread_mutex_unlock_5_1_m;

/** child thread function **/
void * pthread_mutex_unlock_5_1_threaded(void * arg)
{
	int ret;
	ret = pthread_mutex_unlock(&pthread_mutex_unlock_5_1_m);
	if (ret == 0)
	{  UNRESOLVED(ret, "Unlocking a not owned recursive mutex succeeded");  }

	if (ret != EPERM) /* This is a "may" assertion */
		printf("Unlocking a not owned recursive mutex did not return EPERM\n");
	
	return NULL;
}

/** parent thread function **/
int pthread_mutex_unlock_5_1(int argc, char * argv[])
{
	int ret;
	pthread_mutexattr_t ma;
	pthread_t  th;

	pthread_mutex_unlock_output_init();

	#if VERBOSE >1
	printf("Initialize the PTHREAD_MUTEX_RECURSIVE mutex\n");
	#endif
	
	ret = pthread_mutexattr_init(&ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex attribute init failed");  }

	ret = pthread_mutexattr_settype(&ma, PTHREAD_MUTEX_RECURSIVE);
	if (ret != 0)
	{  UNRESOLVED(ret, "Set type recursive failed");  }

	ret = pthread_mutex_init(&pthread_mutex_unlock_5_1_m, &ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex init failed");  }

	#if VERBOSE >1
	printf("Lock the mutex\n");
	#endif
	
	ret = pthread_mutex_lock(&pthread_mutex_unlock_5_1_m);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex lock failed");  }

	/* destroy the mutex attribute object */
	ret = pthread_mutexattr_destroy(&ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex attribute destroy failed");  }

	#if VERBOSE >1
	printf("Create the thread\n");
	#endif
	
	ret = pthread_create(&th, NULL, pthread_mutex_unlock_5_1_threaded, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "Thread creation failed");  }

	/* Let the thread terminate */
	ret = pthread_join(th, NULL);
	if (ret != 0)
	{  UNRESOLVED(ret, "Thread join failed");  }
	
	#if VERBOSE >1
	printf("Joined the thread\n");
	#endif
	
	/* We can clean everything and exit */
	ret = pthread_mutex_unlock(&pthread_mutex_unlock_5_1_m);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex unlock failed. Mutex got corrupted?");  }

	printf("Test PASS\n");
	PASSED;
}
#else /* WITHOUT_XOPEN */
int pthread_mutex_unlock_5_1(int argc, char * argv[])
{
	pthread_mutex_unlock_output_init();
	UNTESTED("This test requires XSI features");
}
#endif

