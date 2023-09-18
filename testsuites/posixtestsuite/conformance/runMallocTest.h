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
	malloc_usable_size_1_1
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
	"malloc_usable_size_1_1"
};

#endif