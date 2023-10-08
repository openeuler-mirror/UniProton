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
 * The function fails and return EPERM if caller has not the 
 * privilege to perform the operation.



 * The steps are:
 * -> if this implementation does not support privileges, return PTS_UNSUPPORTED
 * -> Otherwise, use the implementation features to come to a situation where
 *      pthread_mutex_init should fail because of the privileges, and then check 
 *      that the return code is EPERM.
 * -> return PTS_UNTESTED if the architecture is not present in the test.
 */
 
 /* We are testing conformance to IEEE Std 1003.1, 2003 Edition */
 #define _POSIX_C_SOURCE 200112L

/********************************************************************************************/
/****************************** standard includes *****************************************/
/********************************************************************************************/
 #include <pthread.h>
 #include <stdio.h>
 #include <unistd.h>
 #include <stdlib.h>
 #include <errno.h>
 #include <stdarg.h>
 #include <sys/utsname.h>
 #include <string.h>
 
/********************************************************************************************/
/******************************   Test framework   *****************************************/
/********************************************************************************************/
 #include "../testfrmw.h"
 #include "../testfrmw.c"
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
  * void pthread_mutex_init_output_init()
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

#ifndef PTS_UNSUPPORTED
#define PTS_UNSUPPORTED 4
#endif
#ifndef PTS_UNTESTED
#define PTS_UNTESTED 5
#endif


/********************************************************************************************/
/***********************************    Test case   *****************************************/
/********************************************************************************************/
int pthread_mutex_init_5_2(int argc, char * argv[])
{
	int ret;
	struct utsname un;
	
	pthread_mutex_init_output_init();
	ret = uname(&un);
	if (ret == -1)
	{  UNRESOLVED(errno, "Unable to get Implementation name");  }
	
	#if VERBOSE > 0
	printf("Implementation is: \n\t%s\n\t%s\n\t%s\n", un.sysname, un.release, un.version);
	#endif
	
	/* If we are running Linux */
	if (strcmp(un.sysname, "Linux") == 0 )
	{
		/* Linux does not provide privilege access to pthread_mutex_init function */
		ret = PTS_UNSUPPORTED;
		printf("Linux does not provide this feature\n");
		pthread_mutex_init_output_fini();
		return ret;
	}
	
	/* If we are running AIX */
	if (strcmp(un.sysname, "AIX") == 0 )
	{
		;
	}
	/* If we are running Solaris */
	if (strcmp(un.sysname, "SunOS") == 0 )
	{
		;
	}
	
	printf("This implementation is not tested yet\n");
	pthread_mutex_init_output_fini();
	return PTS_UNTESTED;
}
