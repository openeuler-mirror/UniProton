/*
 * Copyright (c) 2005, Bull S.A..  All rights reserved.
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
 
 
 * This file is a wrapper to use the tests from the NPTL Test & Trace Project
 * with either the Linux Test Project or the Open POSIX Test Suite.
 
 * The following macros are defined here:
 * UNRESOLVED(ret, descr);  
 *    where descr is a description of the error and ret is an int (error code for example)
 * FAILED(descr);
 *    where descr is a short text saying why the test has failed.
 * PASSED();
 *    No parameter.
 * 
 * Both three macros shall terminate the calling process. 
 * The testcase shall not terminate without calling one of those macros.
 * 
 * 
 */
 
#include "posixtest.h"
#include <string.h> /* for the strerror() routine */


#ifdef __GNUC__ /* We are using GCC */

  #define UNRESOLVED(x, s) \
 { printf("Test %s unresolved: got %i (%s) on line %i (%s)\n", __FILE__, x, strerror(x), __LINE__, s); \
 	sem_getvalue_output_fini(); \
 	return PTS_UNRESOLVED ; }
 	
 #define FAILED(s) \
 { printf("Test %s FAILED: %s\n", __FILE__, s); \
 	sem_getvalue_output_fini(); \
 	return PTS_FAIL ; }
 	
 #define PASSED \
  sem_getvalue_output_fini(); \
  return PTS_PASS ;
  
 #define UNTESTED(s) \
{	printf("File %s cannot test: %s\n", __FILE__, s); \
	  sem_getvalue_output_fini(); \
  return PTS_UNTESTED ; \
}
  
#else /* not using GCC */

  #define UNRESOLVED(x, s) \
 { sem_getvalue_output("Test unresolved: got %i (%s) on line %i (%s)\n", x, strerror(x), __LINE__, s); \
  sem_getvalue_output_fini(); \
 	return PTS_UNRESOLVED ; }
 	
 #define FAILED(s) \
 { sem_getvalue_output("Test FAILED: %s\n", s); \
  sem_getvalue_output_fini(); \
 	return PTS_FAIL ; }
 	
 #define PASSED \
  sem_getvalue_output_fini(); \
  return PTS_PASS ;

 #define UNTESTED(s) \
{	sem_getvalue_output("Unable to test: %s\n", s); \
	  sem_getvalue_output_fini(); \
  return PTS_UNTESTED ; \
}

#endif

