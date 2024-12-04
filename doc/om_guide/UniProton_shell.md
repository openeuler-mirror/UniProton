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