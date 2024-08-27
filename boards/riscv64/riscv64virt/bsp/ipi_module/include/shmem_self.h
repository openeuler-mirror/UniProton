#ifndef __SHMEM_H
#define __SHMEM_H

#if defined(CONFIG_SHMEM_SELF)
extern char shmem_start[];
#define SHMEM_BASE	shmem_start   
#define SHMEM_SIZE    	4096
#define PUSH_SHMEM(x, core)  	*(volatile unsigned long long*)(SHMEM_BASE + 8*(core)) = (x);
#define GET_SHMEM(core) 	*(volatile unsigned long long*)(SHMEM_BASE + 8*(core))

#else

#define SHMEM_BASE 		0
#define SHMEM_SIZE   		0
#define PUSH_SHMEM(x, core)  	 
#define GET_SHMEM(core)		0

#endif

/* 
typedef struct shmem_hd_s {
	

} shmem_hd_t;
*/


#endif
