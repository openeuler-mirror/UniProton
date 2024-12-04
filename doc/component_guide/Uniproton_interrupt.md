# Uniproton 中断栈溢出保护使用指南

## 1 整体方案：
目前使用软件保护方案。在分配系统栈（中断栈）时，初始化系统栈时写入魔鬼数字，每次发送中断时，检查本核的系统栈栈顶魔鬼数字是否被改变，若栈顶魔鬼数字被改变，则说明栈溢出，则进入中断栈溢出处理程序。

中断栈溢出时，进入异常，并打印出本核系统栈溢出的提示信息。

## 2 编译时使能中断栈保护功能：
使用中断栈溢出保护功能，需要在defconfig设置
```
CONFIG_OS_OPTION_INTERRUPT_PRO=y
```
## 3 中断栈保护功能适配：
中断栈保护功能接口参考OsSysStackCheck, 系统栈初始化参考InitSystemStack();

**测试套参考:**

testsuites/kern-test/interrupt_test.c

测试套中发送中断，通过中断函数中创建大数组触发中断栈溢出。
```
volatile U32 list[6000] = {0}; // 创建大于系统栈大小的数组，中断栈溢出。
list[100] = 1;
TEST_LOG("[SUCCESS] reccceived test interrupt handler.\n");
```
目前已经在demos/ascend310b/CMakeLists.txt适配测试套UniPorton_test_ir。