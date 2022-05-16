## 系统架构
UniProton系统由Mem、Arch、Kernel、IPC、OM五大子系统构成，Mem、Arch是整个系统的基石。

各子系统的职责如下：
- Mem：实现内存分区的创建，内存块的申请和释放等功能。
- Arch：实现和芯片强相关的硬件特性功能，如硬中断、异常接管等。
- Kernel：实现任务、软中断、TICK中断、软件定时器等功能。
- IPC：实现事件、队列、信号量等功能。
- OM：实现cpup、hook等调测功能。

## 代码目录结构说明

| 一级目录 | 二级目录 | 三级目录 | 说明 |
| ------- | -------- | -------- | -------------------------|
| build | uniproton_ci_lib |           | 编译框架的公共脚本 |
|       | uniproton_config | config_m4 | cortex_m4芯片的功能宏配置文件 |
| cmake | common       | build_auxiliary_script | 转换Kconfig文件为buildef.h脚本 |
|       | functions    |           | cmake的公共功能函数 |
|       | tool_chain   |           | 编译器和编译选项配置文件 |
| doc   |              |           | 项目配置、规范、协议等文档 |
|       | design       |           | UniProton系统架构和特性说明 |
| platform |           |           | libboundscheck使用说明 |
|          | libboundscheck |     | libboundscheck预留目录，用户将下载的源码放在此目录下 |
| src   | arch         | cpu       | cpu对应架构的功能适配代码 |
|       |              | include   | cpu对应架构的头文件 |
|       | config       |           | 用户main函数入口 |
|       |              | config    | 用户配置功能宏开关 |
|       | core         | ipc       | 事件、队列、信号量等功能 |
|       |              | kernel    | 任务、中断、异常、软件定时器等功能 |
|       | include      | uapi      | 对外头文件 |
|       | mem          |           | 内存管理基本功能 |
|       |              | fsc       | 内存管理FSC算法代码 |
|       |              | include   | 内存管理头文件 |
|       | om           | cpup      | cpu占用率统计功能 |
|       |              | err       | 错误处理功能 |
|       |              | hook      | 钩子函数功能 |
|       |              | include   | 系统管理头文件 |
|       | security     | rnd       | 随机化功能 |
|       | utility      | lib       | 公共库函数 |