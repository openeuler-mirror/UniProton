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
 
 
 * This file is a wrapper to use the tests from the NPTL Test & Trace Project
 * with either the Linux Test Project or the Open POSIX Test Suite.
 
 * The following function are defined:
 * void pthread_mutex_unlock_output_init()
 * void pthread_mutex_unlock_output_fini()
 * void pthread_mutex_unlock_output(char * string, ...)
 * 
 * The are used to pthread_mutex_unlock_output informative text (as a printf).
 */
 
/* We use a mutex to avoid conflicts in traces */
static pthread_mutex_t pthread_mutex_unlock_m_trace = PTHREAD_MUTEX_INITIALIZER;

/*****************************************************************************************/
/******************************* stdout module *****************************************/
/*****************************************************************************************/
/* The following functions will pthread_mutex_unlock_output to stdout */
#if (1)
static void pthread_mutex_unlock_output_init()
{
	/* do nothing */
	return;
}
static void pthread_mutex_unlock_output( char * string, ... )
{
   va_list ap;
   pthread_mutex_lock(&pthread_mutex_unlock_m_trace);
   va_start( ap, string);
   vprintf(string, ap);
   va_end(ap);
   pthread_mutex_unlock(&pthread_mutex_unlock_m_trace);
}
static void pthread_mutex_unlock_output_fini()
{
	/*do nothing */
	return;
}
#endif

