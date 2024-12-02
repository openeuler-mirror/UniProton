# Uniproton testsuites使用指南

## 1 Uniproton testsuites用例：
UniProton跟测试相关代码放在UniProton/testsuites目录下，如果想运行对应的测试用例来测试性能和指标，需要编译对应测试套的二进制文件。

## 2 Uniproton testsuites组成架构：
以ascend310b posixtestsuite测试malloc为例，概述测试用例如何运行：

(1) 修改demos/ascend310b/CMakeLists.txt文件，将UniProton/testsuites/posixtestsuite/conformance中将相关的代码加入编译：
```
if (${APP} STREQUAL "UniPorton_test_posix_malloc_interface")
        add_subdirectory(${HOME_PATH}/testsuites/posixtestsuite/conformance tmp)
        target_compile_options(rpmsg PUBLIC -DPOSIX_TESTCASE)
        list(APPEND OBJS $<TARGET_OBJECTS:rpmsg> $<TARGET_OBJECTS:bsp> $<TARGET_OBJECTS:config> $<TARGET_OBJECTS:posixTest>)
```
(2) 修改UniProton/demos/ascend310b/apps/openamp/main.c文件，在创建的任务中添加测试套任务入口：
```
#if defined(POSIX_TESTCASE)
void Init(uintptr_t param1, uintptr_t param2, uintptr_t param3, uintptr_t param4);
#endif

#if defined(POSIX_TESTCASE)
    PRT_Printf("init testcase\n");
    Init(0, 0, 0, 0);
#endif
```
(3) testsuites/posixtestsuite/conformance/CMakeLists.txt，会根据编译的bin文件名称，将对应测试套源码编译：
```
if (${APP} STREQUAL "UniPorton_test_posix_malloc_interface")
    set(BUILD_APP "UniPorton_test_posix_malloc_interface")
    set(ALL_SRC runMallocTest.c ${ALL_MALLOC_SRC})
```

(4)malloc测试用例会将testsuites/posixtestsuite/conformance/runMallocTest.c文件编译到bin文件中,Init入口依次运行run_test_arry_1中的测试用例：
```
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
```

(5) 最后需要修改UniProton/demos/ascend310b/build/build_app.sh中APP，然后sh +x build_app.sh编译即可：
```
export APP=UniPorton_test_posix_malloc_interface
```

## 3 Uniproton testsuites新增测试用例：
整个编译框架已搭建好，如果用户想新增malloc测试用例，在run_test_arry_1数组中新增测试套实现即可。如果想新增其他类型的测试套，需要根据编译选项找到测试套具体执行的Init入口函数，在对应的数组中新增测试套实现加入编译即可。

## 4 Uniproton testsuites用例支持情况：
* posixtestsuite
* rhealstone
* drivers
* shell
* libc
* libxml2
* log
* forte
* cxx
* eigen
* modbus
* soem
