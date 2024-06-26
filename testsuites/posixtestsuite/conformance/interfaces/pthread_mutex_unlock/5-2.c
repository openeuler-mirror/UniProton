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
 * and a thread attempts to unlock an unlocked mutex,
 * an error is returned.

 * The steps are:
 *  -> Initialize a recursive mutex
 *  -> Attempt to unlock the mutex when it is unlocked 
 *      and when it has been locked then unlocked.
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


/** parent thread function **/
int pthread_mutex_unlock_5_2(int argc, char * argv[])
{
	int ret;
	pthread_mutexattr_t ma;
	pthread_mutex_t m;

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

	ret = pthread_mutex_init(&m, &ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex init failed");  }

	#if VERBOSE >1
	printf("Unlock unlocked mutex\n");
	#endif
	
	ret = pthread_mutex_unlock(&m);
	if (ret == 0)
	{  FAILED("Unlocking an unlocked recursive mutex succeeded");  }
	
	#if VERBOSE >1
	printf("Lock and unlock the mutex\n");
	#endif
	
	ret = pthread_mutex_lock(&m);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex lock failed");  }
	ret = pthread_mutex_lock(&m);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex recursive lock failed");  }
	ret = pthread_mutex_unlock(&m);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex unlock failed");  }
	ret = pthread_mutex_unlock(&m);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex recursive unlock failed");  }

	/* destroy the mutex attribute object */
	ret = pthread_mutexattr_destroy(&ma);
	if (ret != 0)
	{  UNRESOLVED(ret, "Mutex attribute destroy failed");  }

	ret = pthread_mutex_unlock(&m);
	if (ret == 0)
	{  FAILED("Unlocking an unlocked recursive mutex succeeded");  }

	printf("Test PASS\n");
	PASSED;
}
#else /* WITHOUT_XOPEN */
int pthread_mutex_unlock_5_2(int argc, char * argv[])
{
	pthread_mutex_unlock_output_init();
	UNTESTED("This test requires XSI features");
}
#endif

