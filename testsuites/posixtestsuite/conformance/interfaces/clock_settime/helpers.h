/*
 * Copyright (c) 2002, Intel Corporation. All rights reserved.
 * Created by:  julie.n.fleischer REMOVE-THIS AT intel DOT com
 * This file is licensed under the GPL license.  For the full content
 * of this license, see the COPYING file at the top level of this
 * source tree.
 *
 * Helper functions used to reset the time to close to the current
 * time at the end of the test.
 *
 * Since these helper functions are made specifically to be included
 * in certain tests, they make use of some libraries already included
 * by those tests.
 */

#ifndef CLOCK_SETTIME_GET_BEFORE_TIME 
#define CLOCK_SETTIME_GET_BEFORE_TIME getBeforeTime
#endif

#ifndef CLOCK_SETTIME_SET_BEFORE_BACK 
#define CLOCK_SETTIME_SET_BEFORE_BACK setBackTime
#endif

int CLOCK_SETTIME_GET_BEFORE_TIME(struct timespec *tpget)
{
	if (clock_gettime(CLOCK_REALTIME, tpget) != 0) {
		perror("clock_gettime() did not return success\n");
		perror("clock may not be reset properly\n");
		return PTS_UNRESOLVED;
	}
	return PTS_PASS;
}

int CLOCK_SETTIME_SET_BEFORE_BACK(struct timespec tpset)
{
	if (clock_settime(CLOCK_REALTIME, &tpset) != 0) {
		perror("clock_settime() did not return success\n");
		perror("clock may not be reset properly\n");
		return PTS_UNRESOLVED;
	}
	return PTS_PASS;
}

