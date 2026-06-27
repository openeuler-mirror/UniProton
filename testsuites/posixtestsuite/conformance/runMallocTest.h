#ifndef _CONFORMSNCE_RUN_TEST_H
#define _CONFORMSNCE_RUN_TEST_H

extern int malloc_calloc_1_1();
extern int malloc_malloc_1_1();
extern int malloc_malloc_2_1();
extern int malloc_malloc_3_1();
extern int malloc_memalign_1_1();
extern int malloc_memalign_2_1();
extern int malloc_realloc_1_1();
extern int malloc_realloc_2_1();
extern int malloc_realloc_3_1();
extern int malloc_reallocarray_1_1();
extern int malloc_usable_size_1_1();

typedef int test_run_main();

test_run_main *run_test_arry_1[] = {
	malloc_calloc_1_1,
	malloc_malloc_1_1,
	malloc_malloc_2_1,
	malloc_malloc_3_1,
	malloc_memalign_1_1,
	malloc_memalign_2_1,
	malloc_realloc_1_1,
	malloc_realloc_2_1,
	malloc_realloc_3_1,
	malloc_reallocarray_1_1,
#ifndef OS_MEM_ARITH_TLSF
	/* malloc_usable_size_1_1 校验的是 FSC 的 tail magic (OS_FSC_MEM_TAIL_MAGIC)，
	   属 FSC 专有内存布局；TLSF 算法无 tail magic 机制，为保证算法原有行为，
	   TLSF 模式下不跑该用例。 */
	malloc_usable_size_1_1,
#endif
};

char run_test_name_1[][50] = {
	"malloc_calloc_1_1",
	"malloc_malloc_1_1",
	"malloc_malloc_2_1",
	"malloc_malloc_3_1",
	"malloc_memalign_1_1",
	"malloc_memalign_2_1",
	"malloc_realloc_1_1",
	"malloc_realloc_2_1",
	"malloc_realloc_3_1",
	"malloc_reallocarray_1_1",
#ifndef OS_MEM_ARITH_TLSF
	"malloc_usable_size_1_1",
#endif
};

#endif