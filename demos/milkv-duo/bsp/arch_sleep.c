#include "arch_time.h"
#include "prt_config.h"

int arch_usleep(unsigned long useconds)
{
	unsigned long start_time;
	unsigned long end_time;
	unsigned long run_time;

	start_time = GetSysTime();
	end_time = start_time + useconds * (OS_SYS_CLOCK / 1000000);
	do{
		run_time = GetSysTime();
	} while(run_time < end_time);
}

int arch_nsleep(unsigned long nseconds)
{
	unsigned long start_time;
	unsigned long end_time;
	unsigned long run_time;

	start_time = GetSysTime();
	end_time = start_time + nseconds;
	do{
		run_time = GetSysTime();
	} while(run_time < end_time);
}
