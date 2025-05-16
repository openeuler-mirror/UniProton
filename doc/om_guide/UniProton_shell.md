# Uniproton Shell使用指南

## 1 整体方案：
当前UniProton shell功能基于mica实现。在Linux侧会初始化一个名为"rpmsg-tty"的rpmsg endpoint,并通过screen命令打开一个tty终端，用于接收并回显RTOS侧的消息。
RTOS侧同样初始化一个名为"rpmsg-tty"的rpmsg endpoint，用于RTOS侧收发消息。当Linux在tty终端输入命令并回车时，命令会通过rpmsg endpoint发送给RTOS的shell字符模块进行解析，字符串解析模块会匹配查找shell命令库，如果在shell命令库中查找到的命令则会执行，并将执行结果在tty终端回显，如果匹配不到命令，tty中断会回显"shell command not found"。

## 2 编译时使能shell功能：
defconfig 设置
```
CONFIG_CONFIG_LOCAL_ECHO=y
CONFIG_LOSCFG_SHELL=y
CONFIG_LOSCFG_SHELL_MICA_INPUT=y
```

## 3 目前支持的shell命令：
* help
* cpup
* hwi
* memInfo
* queue
* sem
* swtmr
* systeminfo
* taskInfo
* uname

## 4 动态添加shell命令：
如果需要添加自定义命令行及参数，可通过osCmdReg接口注册：
```
static int osShellCmdTstReg(int argc, const char **argv)
{
    printf("tstreg: get %d arguments\n", argc);
    for(int i = 0; i < argc; i++) {
        printf("    no %d arguments: %s\n", i + 1, argv[i]);
    }

    return 0;
}

osCmdReg(CMD_TYPE_EX, "tstreg", XARGS, (CMD_CBK_FUNC)osShellCmdTstReg);
```
(1)参数对应意义如下：
```

CMD_TYPE_EX: 命令行类型

"tstreg": 命令行名字

XARGS: 运行函数的参数

osShellCmdTstReg: 命令行回调运行函数
```

(2)运行情况如下：
```
openEuler UniProton # tstreg 1 abc
tstreg: get 2 arguments
    no 1 arguments: 1
    no 2 arguments: abc
```

## 5 性能测试shell命令：
新增性能测试rhealstone与cyclictest的shell命令，在编译时，需打开性能测试命令行支持开关：
```
CONFIG_LOSCFG_SHELL_TEST=y
```

性能测试命令具体使用方式如下：
1) rhealstone命令的使用方式为：
```
rhealstone [testcase]
```
其中参数testcase代表具体测试的用例名称，包含deadlock-break, interrupt-latency, message-latency, semaphore-shuffle, task-preempt, task-switch六个用例。

2) cyclictest命令的使用方式为：
```
cyclictest [-i] [interval] [-l] [loopNum]
```
其中可选参数-i设置了每次循环中任务睡眠的时间间隔，单位为微秒，默认值为1000；可选参数-l设置了任务的循环次数，默认值为1000。

## 6 perf功能shell命令：
新增perf功能的shell命令，在编译时，需打开perf功能开关和perf功能命令行支持开关：
```
CONFIG_OS_OPTION_PERF=y
CONFIG_LOSCFG_SHELL_PERF=y
```
除以上两个开关，还需要根据所选的perf监测方式打开对应的监测开关，当前UniProton共支持硬件监测、软件监测、定时器监测三种方式，但每次使用只能选择一种监测方式：
```
CONFIG_OS_OPTION_PERF_HW_PMU=y     # 使能硬件监测
CONFIG_OS_OPTION_PERF_SW_PMU=y     # 使能软件监测
CONFIG_OS_OPTION_PERF_TIMED_PMU=y  # 使能定时器监测
```

perf命令具体使用方式如下：
```
perf [-i] [interval]
```
其中可选参数-i设置了本次监测的时长，单位为微秒，如果不设置该参数可直接执行perf命令，对应的默认监测时长为1000微秒。执行命令后，会显示各硬件或软件事件的统计值。当使用定时器监测时，会在串口打印采样信息，采样信息同时会保存到单板的/tmp/output.perf文件中，利用FlameGraph工具可将该文件中的信息转化为火焰图。火焰图的具体生成方法可见[perf使用指南](./perf.md)。

## 7 日志功能shell命令：
新增日志功能相关shell命令，在编译时，需打开日志功能开关和日志命令行支持开关：
```
CONFIG_OS_OPTION_LOG=y
CONFIG_LOSCFG_SHELL_LOG=y
```
在非实时侧同时适配日志功能，适配步骤可见[日志介绍](./log.md)

日志命令行支持日志的开关和等级设置，具体使用方式如下：
```
log on           // 打开日志功能
log off          // 关闭日志功能
log status       // 查看当前日志状态，包括日志的开关情况和日志等级
log set [level]  // 设置日志级别，级别值 1-8
```
当前日志级别从高到低共有"EMERG", "ALERT", "CRIT", "ERR", "WARN", "NOTICE", "INFO", "DEBUG"八级，分别对应级别值0-7，在通过log set命令设置日志级别时，所设级别和其之下的级别将被过滤，因此级别0不可被设置。当级别值设置为8时，表示所有级别的日志都可输出。
