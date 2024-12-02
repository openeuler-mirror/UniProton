# UniProton 代码&文档贡献指南
-   [编程规范](#编程规范)
    - [总体原则](#总体原则)
    - [目录结构](#目录结构)
    - [命名](#命名)
    - [排版与格式](#排版与格式)
    - [注释](#注释)
    - [宏](#宏)
    - [头文件](#头文件)
    - [数据类型](#数据类型)
    - [变量](#变量)
    - [函数](#函数)
    - [可移植性](#可移植性)
    - [业界编程规范](#业界编程规范)
-   [文档写作规范](#文档写作规范)
-   [Commit message规范](#Commitmessage规范)
-   [贡献流程](#贡献流程)
-   [协议](#协议)
-   [加入我们](#加入我们)

## 编程规范

此编程规范在业界通用的编程规范基础上进行了整理，供开发者参考使用。

### 总体原则
-   清晰，易于维护、易于重构。
-   简洁，易于理解，并且易于实现。
-   风格统一，代码整体风格保持统一。
-   通用性，遵循业界通用的编程规范。

### 目录结构
建议将工程按照功能模块划分子目录（可参考UniProton的功能模块划分），子目录再定义头文件和源文件目录。

### 命名
- 使用驼峰风格进行命名，此风格大小写字母混用，不同单词间通过单词首字母大写来分开，具体规则如下：
    |           类型              |         命名风格                 |         形式              |
    | --------------------------- | -------------------------------- | ------------------------- |
    | 函数，自定义的类型          |  大驼峰，或带有模块前缀的大驼峰  | AaaBbb, XXX_AaaBbb        |
    | 局部变量，函数参数，宏参数，结构体成员，联合体成员  |  小驼峰  | aaaBbb                    |
    | 全局变量                    |  带'g_'前缀的小驼峰              | g_aaaBbb                  |
    | 宏，枚举值                  |  全大写并下划线分割              | AAA_BBB                   |
    | 内核头文件中防止重复包含的宏变量  |  带'PRT'前缀和'H'后缀，中间为大写模块名，以下划线分割 | PRT_MODULE_H  |
- 全局函数、全局变量、宏、类型名、枚举名的命名，应当准确描述并全局唯一。
- 在能够准确表达含义的前提下，局部变量，或结构体、联合体的成员变量，其命名应尽可能简短。
- UniProton 的对外 API 使用 `PRT_<Module><Func>`的方式，如果有动词和宾语，则采用 `PRT_<Module><Verb><Object>`，比如：
    ```
    PRT_TaskCreate
    PRT_SemPend
    PRT_TickGetCount
    PRT_TaskGetStatus
    ```
   kernel 目录下内部模块间接口使用 `Os<Module><Func>` 的方式，比如：
    ```
    OsTaskScan
    OsSwTmrCtrlInit
    ```

### 排版与格式
-   程序块采用缩进风格编写，使用空格而不是制表符（'\t'）进行缩进，每级缩进为4个空格。
-   采用K&R风格作为大括号换行风格，即函数左大括号另起一行放行首，并独占一行，其他左大括号跟随语句放行末，
    右大括号独占一行，除非后面跟着同一语句的剩余部分，如if语句的else/else if或者分号，比如：
    
    ```c
    struct MyType {   // 左大括号跟随语句放行末，前置1个空格
        ...
    };                // 右大括号后面紧跟分号
    ```
    ```c
    int Foo(int a)
    {                 // 函数左大括号独占一行，放行首
        if (a > 0) {  // 左大括号跟随语句放行末，前置1个空格
            ...
        } else {      // 右大括号、"else"、以及后续的左大括号均在同一行
            ...
        }             // 右大括号独占一行
        ...
    }
    ```
-   条件、循环语句使用大括号，比如：
    ```c
    if (objectIsNotExist) { // 单行条件语句也加大括号
        return CreateNewObject();
    }
    ```
    ```c
    while (condition) {} // 即使循环体是空，也应使用大括号
    ```
    ```c
    while (condition) {
        continue;        // continue表示空逻辑，使用大括号
    }
    ```
-   case/default语句相对switch缩进一层，缩进风格如下：
    ```c
    switch (var) {
        case 0:             // 缩进一层
            DoSomething1(); // 缩进一层
            break;
        case 1:
            DoSomething2();
            break;
        default:
            break;
    }
    ```
-   一行只写一条语句。
-   一条语句不能过长，建议不超过120个字符，如不能缩短语句则需要分行写。
-   换行时将操作符留在行末，新行进行同类对齐或缩进一层，比如：
    ```c
    // 假设下面第一行不满足行宽要求
    if (currentValue > MIN && // 换行后，布尔操作符放在行末
        currentValue < MAX) { // 与(&&)操作符的两个操作数同类对齐
        DoSomething();
        ...
    }
    ```
    ```c
    // 假设下面的函数调用不满足行宽要求，需要换行
    ReturnType result = FunctionName(paramName1,
                                     paramName2,
                                     paramName3); // 保持与上方参数对齐
    ```
    ```c
    ReturnType result = VeryVeryVeryLongFunctionName( // 写入第1个参数后导致过长，直接换行
        paramName1, paramName2, paramName3);          // 换行后，4空格缩进一层
    ```
    ```c
    // 每行的参数代表一组相关性较强的数据结构，放在一行便于理解，此时可理解性优先于格式排版要求
    int result = DealWithStructLikeParams(left.x, left.y,    // 表示一组相关参数
                                          right.x, right.y); // 表示另外一组相关参数
    ```
-   声明定义函数时，函数的返回类型以及其他修饰符，与函数名同行。
-   指针类型"*"应该靠右跟随变量或者函数名，比如：
    ```c
    int *p1;  // Good：右跟随变量，和左边的类型隔了1个空格
    int* p2;  // Bad：左跟随类型
    int*p3;   // Bad：两边都没空格
    int * p4; // Bad：两边都有空格
    ```
    当"*"与变量或函数名之间有其他修饰符，无法跟随时，此时也不要跟随修饰符，比如：
    ```c
    char * const VERSION = "V100";    // Good：当有const修饰符时，"*"两边都有空格
    int Foo(const char * restrict p); // Good：当有restrict修饰符时，"*"两边都有空格
    ```
-   根据上下内容的相关程度，合理安排空行，但不要使用连续3个或更多空行。
-   编译预处理的"#"统一放在行首，无需缩进。嵌套编译预处理语句时，"#"可以进行缩进，比如：
    ```c
    #if defined(__x86_64__) && defined(__GCC_HAVE_SYNC_COMPARE_AND_SWAP_16) // 位于行首，不缩进
        #define ATOMIC_X86_HAS_CMPXCHG16B 1                                 // 缩进一层，区分层次，便于阅读
    #else
        #define ATOMIC_X86_HAS_CMPXCHG16B 0
    #endif
    ```

### 注释
-   注释的内容要清楚、明了，含义准确，防止注释二义性。
-   在代码的功能、意图层次上进行注释，即注释解释代码难以直接表达的意图，而不是仅仅重复描述代码。
-   函数声明处注释描述函数功能、性能及用法，包括输入和输出参数、函数返回值、可重入的要求等；定义处详细描述函数功能和实现要点，如实现的简要步骤、实现的理由、设计约束等。
-   全局变量要有较详细的注释，包括对其功能、取值范围以及存取时注意事项等的说明。
-   避免在注释中使用缩写，除非是业界通用或子系统内标准化的缩写。
-   文件头部要进行注释，建议注释列出：版权说明、版本号、生成日期、作者姓名、功能说明、与其它文件的关系、修改日志等。
-   注释风格要统一，建议优先选择/* */的方式，注释符与注释内容之间要有1空格，单行、多行注释风格如下：
    ```c
    /* 单行注释 */
    ```
    ```c
    /*
     * 多行注释
     * 第二行
     */
    ```
-   注释应放在其代码上方或右方。

    上方的注释，与代码行之间无空行，保持与代码一样的缩进。
    右边的注释，与代码之间至少相隔1个空格。如果有多条右置注释，上下对齐会更加美观，比如：
    ```c
    #define A_CONST 100         // 此处两行注释属于同类
    #define ANOTHER_CONST 200   // 可保持左侧对齐
    ```

### 宏
-   代码片段使用宏隔离时，统一通过#ifdef的方式，例如：
    ```c
    #ifdef PRTCFG_XXX
    ...
    #endif
    ```
-   定义宏时，要使用完备的括号，比如:
    ```c
    #define SUM(a, b) a + b       // 不符合本条要求
    #define SUM(a, b) ((a) + (b)) // 符合本条要求
    ```
    但是也要避免滥用括号，比如单独的数字或标识符加括号毫无意义：
    ```c
    #define SOME_CONST  100         // 单独的数字无需括号
    #define ANOTHER_CONST   (-1)    // 负数需要使用括号
    #define THE_CONST   SOME_CONST  // 单独的标识符无需括号
    ```
-   包含多条语句的函数式宏的实现语句必须放在do-while(0)中，例如：
    ```c
    #define FOO(x) do { \
        (void)printf("arg is %d\n", (x)); \
        DoSomething((x)); \
    } while (0)
    ```
-   禁止宏调用参数中出现预编译指令。
-   宏定义不以分号结尾。

### 头文件
-   设计原则
    -   头文件应当职责单一。
    -   一个模块通常包含多个.c文件，建议放在同一个目录下，目录名即为模块名；如果一个模块包含多个子模块，则建议每一个子模块提供一个对外的.h，文件名为子模块名。
    -   建议每一个.c文件应有一个同名.h文件，用于声明需要对外公开的接口。
    -   头文件中适合放置接口的声明，不适合放置实现。
    -   不要在头文件中定义变量。
    -   禁止头文件循环依赖，循环依赖指a.h包含b.h，b.h包含c.h，c.h包含a.h。
    -   头文件应当自包含，即任意一个头文件均可独立编译，但同时也要避免包含用不到的头文件。
    -   头文件必须用#deﬁne保护，防止重复包含，比如内核中统一使用以下宏定义保护：
        ```c
        #ifndef PRT_<MODULE>_H  // 比如 PRT_TASK_H
        #define PRT_<MODULE>_H
        ...
        #endif
        ```
    -   禁止通过声明的方式引用外部函数接口、变量，只能通过包含头文件的方式使用其他模块或文件提供的接口。
    -   禁止在 extern "C" 中包含头文件。
    -   按照合理的顺序包含头文件：
        1) 源文件对应的头文件
        2) C标准库
        3) 需要包含的OS其他头文件
-   版权声明
    -   头文件版权声明一致，放在头文件置顶位置。
    -   如果提交的代码是在开源软件基础上修改所编写或衍生的代码，请遵循开源许可协议要求，并且已履行被修改软件的许可证义务。

### 数据类型
基础类型定义统一使用prt_typedef.h中定义的类型，比如定义无符号32位整型变量使用U32。

### 变量
-   一个变量只有一个功能，不要把一个变量用作多种用途。
-   防止局部变量与全局变量同名。
-   不用或者少用全局变量。
-   定义函数的局部变量时，控制变量的占用空间，避免因占用过多栈空间导致程序运行失败。比如需要一个大数组，可以通过动态分配内存的方式来避免栈空间占用过大。
-   在首次使用前初始化变量。
-   指向资源句柄或描述符的变量，在资源释放后立即赋予新值，包括指针、文件描述符以及其它指向资源的变量。
-   禁止将局部变量的地址返回到其作用域以外，下面是一个**错误示例：**
    ```c
    int *Func(void)
    {
        int localVar = 0;
        ...
        return &localVar;  // 错误
    }
    
    void Caller(void)
    {
        int *p = Func();
        ...
        int x = *p;       // 程序产生未定义行为
    }
    ```
    **正确代码示例：**
    ```c
    int Func(void)
    {
        int localVar = 0;
        ...
        return localVar;
    }
    
    void Caller(void)
    {
        int x = Func();
        ...
    }
    ```
-   如果要使用其他模块的变量，应尽量避免直接对变量进行访问，而是通过统一的函数封装或者宏封装的方式，比如mutex模块中：
    ```c
    // 私有头文件中引入全局变量，但要避免直接使用
    extern struct TagQueCb *g_allQueue;
    // 通过GET_QUEUE_HANDLE的方式对g_allQueue进行访问
    #define GET_QUEUE_HANDLE(queueId) (((struct TagQueCb *)g_allQueue) + (queueId))
    ```

### 函数
-   重复代码应该尽可能提炼成函数。
-   避免函数过长，新增函数不超过 40-50 行。
-   内联函数要尽可能短，避免超过 10 行（非空非注释）。
-   避免函数的代码块嵌套过深。
-   函数应避免使用全局变量、静态局部变量和I/O操作，不可避免的地方应集中使用。

### 可移植性
不使用与硬件或操作系统关系很大的语句，而使用建议的标准语句，以提高软件的可移植性和可重用性。

### 业界编程规范
C语言编程规范参考资料较多，大家可以自行了解，本文不再过多赘述。

## 文档写作规范

UniProton欢迎开发者参与到开源社区的贡献中来，本文主要介绍参与UniProton文档贡献的写作规范，如果贡献者提交文档的修改或提交新的文档，请参照此规范。

### 命名规范

如需提交新的文档，在<a href="https://gitee.com/openeuler/UniProton" target="_blank">UniProton代码仓</a>doc目录下创建新的.md文件，命名需遵循 xxx\_xxx.md 格式，根据文档的内容来声明。

比如介绍写作规范的文档，可以命名为 UniProton\_doc\_write\_standard.md。

### 内容规范

以简洁、直观地表达所述内容为目的，介绍性文档言简意赅介绍原理、架构、设计思路等，操作类文档写明关键步骤，以便能对其他开发者起到帮助。可以优先使用中文，建议中英文都支持，UniProton也将持续更新，保证中英文的同步。

1.  **标题**

    建议标题层级不超过三级。

2.  **正文**
    -   操作类文档以移植为例，文档结构可以参考如下：
        1.  目的（简述操作目的，如移植到哪款型号的单板）
        2.  软硬件环境准备
        3.  移植具体步骤
        4.  结果验证

    -   介绍性文档以开发指南某一功能为例，文档结构可以参考如下：
        1.  概述（概念及原理介绍）
        2.  功能（支持的接口列表）
        3.  开发流程（如何使用及相应步骤）
        4.  编程实例（提供具体代码示例）
        5.  注意事项
        6.  其他

3.  **图片**

    图片统一存放到文档同级目录下的images文件夹中（英文文档对应images-en），如 UniProton/doc/getting\_started.md 中使用的图片，统一放置到  UniProton/doc/images 目录下，文档中使用相对路径引用图片。图片建议根据内容命名，只用数字序列不利于后续图片的继承。
    <!-- 
    >![](public_sys-resources/icon-note.gif) **说明：** 
    >引用方式：
    >!\[\]\(./images/images-standard.png\)
    -->
    **说明：**  图  
    如果是自制图片，配色请参考如下，格式不限，png/jpg/gif...均可，如果是截图或者其他地方引用图片，风格不做限制。
    <!-- 
    ![](images/contribute/picture_color_in_document_writing.png)
    -->
    **说明：**  图  
4.  **表格**

    在md文件中可以按照如下形式插入表格。

    ```
    | Tables      | Type          | Note  |
    | ----------- |:-------------:| -----:|
    | first       | standard      |  None |
    | second      | outstanding   |     5 |
    | third       | inside        |  with |
    ```

5.  **代码**

    正文中插入代码段，可以在段落前后加上符号 \`\`\`。
    <blockquote>
    ```c

    int size = 10;

    \```
    </blockquote>

## Commit message规范

### 概要说明

目前，社区有多种 Commit message 的写法规范。UniProton采用的是Angular规范，这是目前使用最广的写法，比较合理和系统化，并且有配套的工具。

### Commit message的作用

格式化的Commit message有几个好处：

- 提供更多的历史信息，方便快速浏览
- 可以过滤某些commit（比如文档改动），便于快速查找信息
- 可以直接从commit生成Change log

### UniProton Commit message的格式

每次提交，Commit message 都包括三个部分：Header，Body 和 Footer。
```
<header>
空一行
<body>
空一行
<footer>
```

比如：

```
fix(stm32f411): fix stm32f411 migration guide file error

fix some error in stm32f411re migration guide file.

Close #75
```

-   **Header格式**

    Header部分只有一行，格式为：

    ```
    <type>(<scope>): <subject>
    ```

    Header部分共包括三个字段：type（必需）、scope（可选）和subject（必需）。

    -   type

        type用于说明 commit 的类别，只允许使用下面7个标识。

        ```
        feat：feature的缩写，表示这是一个新功能
        fix：修补bug
        docs：documentation的缩写，表示修改的是文档
        style： 格式修改，是不影响代码运行的变动
        refactor：重构，它即不是新增功能，也不是修改bug
        test：增加测试
        chore：构建过程或辅助工具的变动
        ```

    -   scope

        scope用于说明 commit 影响的范围，比如对UniProton Kernel的base的修改会影响全部代码，所以填写all。如果只修改STM32F411的代码，则填写STM32F411。

    -   subject

        subject用于简短描述 commit 目的，不超过50个字符。subject以动词开头，使用第一人称现在时，比如change，而不是changed或changes。subject的第一个字母小写，结尾不加英文句号（.）。

-   **Body格式**

    Body 部分是对本次 commit 的详细描述，可以分成多行，下面是一个范例。

    ```
    Add porting contest board projects to UniProton
    Board list:
    Arduino-M0-PRO
    ATSAM4S-XPRO
    ATSAMD21-XPRO
    EFM32-SLSTK3400A
    EFM32-SLSTK3401A
    EFM32-STK3700
    FRDM-KL26Z
    FRDM-KW41Z
    ```

    有两个注意点：

    -   使用第一人称现在时，比如使用change而不是changed或changes。
    -   应该说明代码变动的动机，以及与以前行为的对比。

-   **Footer格式**

    Footer 部分只用于两种情况。

    -   不兼容变动

        如果当前代码与上一个版本不兼容，则 Footer 部分以BREAKING CHANGE开头，后面是对变动的描述、以及变动理由和迁移方法。

        ```
        BREAKING CHANGE: isolate scope bindings definition has changed.
        To migrate the code follow the example below:
        Before:
        scope: {
            myAttr: 'attribute',
        }
        After:
        scope: {
            myAttr: '@',
        }
        The removed `inject` wasn't generaly useful for directives so there should be no code using it.
        ```

    -   关闭 Issue

        如果当前 commit 针对某个issue，那么可以在 Footer 部分关闭这个 issue 。

        ```
        Closes #16, #24, #92
        ```

### 更多参考

更详细的 commit 规则请参考原始的规范说明：[Angular规范](https://github.com/mychaser/docgather/blob/master/GitCommitMessageConventions.pdf)。

## 贡献流程

UniProton的代码仓托管在gitee上，因此代码贡献者需要在gitee上注册账号才能贡献代码。注册账号可以参考<a href="https://gitee.com/help/articles/4113#article-header0" target="_blank">注册Gitee账号</a>。

代码贡献可以分为**在线修改**和**本地提交**两种。

### 方法一：在线修改

在线修改适用于修改量较少的情况，方便快捷，点击页面“编辑”按钮，跳转到对应的编辑页面。
<!-- 
![](./images/contribute/gitee_online_edit.png)
-->
**说明：**  图  
编辑修改后，点击页面下方“提交”按钮，即提交修改到 UniProton 工程，静候工作人员审核即可。
<!-- 
![](./images/contribute/gitee_online_submit.png)
-->
**说明：**  图  

### 方法二：本地提交

本地提交适用于修改量较大的情况，主要说明如何参与UniProton开源代码贡献。进行UniProton的代码贡献可以遵循以下流程：

1.  下载Git工具。
2.  配置SSH公钥。
3.  配置本地Git账户信息。
4.  fork UniProton源代码。
5.  同步UniProton仓库代码到fork的仓库。
6.  提交本地修改到fork的仓库。
7.  提交Pull Request到UniProton官方主仓库。
8.  查看Pull Request的状态。

#### **1 下载Git工具**
请至<a href="https://git-scm.com/download" target="_blank">git官网下载</a>，安装方法可以参考<a href="https://gitee.com/help/articles/4106#article-header0" target="_blank">Git的安装</a>。

#### **2 配置SSH公钥**
1\) 先查看本地是否有公钥。如果存在公钥，则返回类似如下显示的一对文件，一般文件名为 id\_rsa 和 id\_rsa.pub，其中 .pub 后缀的文件是公钥，另一个文件是密钥。

```
$ ls ~/.ssh
id_rsa id_rsa.pub
```

如果系统内没有公钥文件，可以执行如下命令生成，公钥文件一般默认存放在 \~/.ssh路径下：

```
$ ssh-keygen -t rsa -C "your-email@youremail.com"  //其中的邮箱为注册gitee账户时使用的邮箱
```

2\) 配置gitee账户的SSH公钥，参考<a href="https://gitee.com/help/articles/4191#article-header0" target="_blank">gitee上SSH公钥设置</a>。

#### **3 配置本地Git账户信息**
账户信息即用户名和邮箱，注意此处的用户名和邮箱指的是注册gitee账户所使用的账户名和邮箱。配置以后，每次Git提交都会使用该信息：

```
$ git config --global user.name "your-username"
$ git config --global user.email "your-email@youremail.com"
```

如果本地pc上已经存储了账户信息，可以在“控制面板-\>用户帐户-\>凭据管理器”中确认保存的gitee账号密码是否正确，如果错误会导致无法提交修改到远程仓库。如果无法在本地pc上确认或修改为正确的账号信息，可以执行如下命令，不使用本地pc中保存的账号密码：

```
$ git config --global --unset credential.helper
```

查看git的所有配置信息，可以使用如下命令：

```
$ git config --list
```

#### **4 fork UniProton源代码**
1\) 使用个人gitee账号登陆gitee。

2\) 进入UniProton官方主仓库（master分支）：<a href="https://gitee.com/openeuler/UniProton" target="_blank">UniProton源码仓</a>。

3\) 点击右上角fork按钮，将UniProton的代码fork到个人账号下，在弹出的窗口中选择要fork到的个人账号，点击确认后，稍等一会就会自动跳转到刚刚fork出来的个人账号下的UniProton仓库。
<!-- 
![](./images/contribute/gitee_fork.png)
**说明：** 图  
-->

#### **5 同步UniProton仓库代码到fork的仓库**
开发代码前，首先需要确保当前个人账号下的UniProton代码和UniProton官方仓库是一致的。 因为从fork代码到现在，UniProton官方仓库可能已经更新了内容，所以开发代码前需要先同步UniProton仓库代码到fork的仓库。如果仓库刚刚fork，可以跳过此步。
<!-- 
![](./images/contribute/gitee_fork4.png)
**说明：** 图 
-->

点击上图中红框中的按钮从UniProton官方仓库拉取代码到个人账号fork的仓库，此时会弹出一个对话框以确定同步动作，如下图所示：
<!-- 
![](./images/contribute/gitee_fork5.png)
**说明：** 图 
-->

点击确定后，gitee就会开始同步代码，用户无需再做其他操作。

#### **6 提交本地修改到fork的仓库**
1\) 开发的第一步，是clone代码到本地pc。

```
git clone https://gitee.com/gitee账户名/UniProton.git    //个人账号fork的仓库地址，clone下来后仓库名默认为origin
```

基于UniProton的远程master分支，创建并切换到本地分支，例如本地分支名也是master：

```
git checkout -b master origin/master
```

2\) 在本地分支上进行开发。开发完成之后，git add 添加代码到本地仓库，并git commit 提交到本地仓库，具体commit信息的填写规范，请参考[Commit message规范](#Commitmessage规范)。

3\) 执行git push origin master操作，将代码提交到gitee上自己个人账号的master分支。

**说明：** 所有git命令相关操作，如果不熟悉，请自行上网查找。

#### **7 提交Pull Request到UniProton官方主仓库**

通过上述步骤，修改已经提交到个人远程仓库中，此时就可以向UniProton官方主仓库master分支提交Pull Request，该操作在gitee网页上进行。

1\) 进入个人账号下fork的UniProton仓库首页，点击下图红框中的“+ Pull Request”。
<!-- 
![](./images/contribute/gitee_fork6.png)
**说明：** 图
-->

2\) 之后gitee会跳转到创建Pull Request的详细页面，并给出对应的源分支和要修改的目标分支，目标分支为UniProton官方主仓库master分支，如下图。
<!--
![](./images/contribute/gitee_fork7.png)
**说明：** 图
-->

如果代码没有冲突则会显示下图红框中“可自动合并”的提示，否则需要先解决冲突然后再重新创建Pull Request。在线解决代码冲突可以参考<a href="https://gitee.com/help/articles/4305" target="_blank">在线解决代码冲突</a>。
<!-- 
![](./images/contribute/gitee_fork8.png)
**说明：** 图
-->

填入Pull Request的标题和说明，点击“创建”，就可以提交一个Pull Request。右边的审查人员、测试人员、里程碑、标签、优先级是可选项，不选择也不影响Pull Request的创建。
<!-- 
>![](public_sys-resources/icon-note.gif)
**说明：** 图
-->

**说明：** 
>-   如果提交的代码是为了解决issue问题，记得将issue和此次代码提交相关联，关联方法请参考<a href="https://gitee.com/help/articles/4141" target="_blank">Commit关联Issue</a>和<a href="https://gitee.com/help/articles/4142" target="_blank">Pull Request关联Issue</a>。
>-   如果提交的Pull Request中有新增意见，需要在评论里回复，并@提意见的人说明已经解决。


#### **8 查看Pull Request的状态**
1\) 进入<a href="https://gitee.com/openeuler/UniProton" target="_blank">UniProton主仓库</a>首页。

2\) 点击下图中的“Pull Requests”，可以看到当前UniProton主仓库上所有的Pull Request。
<!-- 
![](./images/contribute/gitee_pr.png)
**说明：** 图 
-->

“开启的”表示这个Pull Request的代码还没有合入，“已合并”表示这个Pull Request的代码已经合入，“已关闭”表示这个Pull Request虽然已经关闭但是代码没有被合入。

现在就静候UniProton主仓库管理员review代码吧，验证ok就会合入修改，恭喜您成为Contributor，感谢您为开源社区做出的贡献。

## 协议

### 知识共享许可协议

**您可以自由地：**

**分享** — 在任何媒介以任何形式复制、发行本文档。

**演绎** — 修改、转换或以本文档为基础进行创作。

只要你遵守许可协议条款，许可人就无法收回你的这些权利。

**惟须遵守下列条件：**

**署名** — 您必须提供适当的证书，提供一个链接到许可证，并指示是否作出更改。您可以以任何合理的方式这样做，但不是以任何方式表明，许可方赞同您或您的使用。

**非商业性使用** — 您不得将本文档用于商业目的。

**相同方式共享** — 如果您的修改、转换，或以本文档为基础进行创作，仅得依本素材的授权条款来散布您的贡献作品。

**没有附加限制** — 您不能增设法律条款或科技措施，来限制别人依授权条款本已许可的作为。

**声明：**

当您使用本素材中属于公众领域的元素，或当法律有例外或限制条款允许您的使用，则您不需要遵守本授权条款。

未提供保证。本授权条款未必能完全提供您预期用途所需要的所有许可。例如：形象权、隐私权、著作人格权等其他权利，可能限制您如何使用本素材。
<!-- 
>![](public_sys-resources/icon-notice.gif) 
说明：图  
-->
**须知：** 
>为了方便用户理解，这是协议的概述。您可以访问网址http://license.coscl.org.cn/MulanPSL2/index.html 了解完整协议内容。

### 知识产权政策

1.  定义

    **1.1  关联公司：** 是指就一个实体而言，该实体直接或间接控制的另一实体，或者，直接或间接控制该实体的另一实体，或者与该实体被某一实体共同控制的其他实体；这里的“控制”是指直接或间接拥有一个实体中多于50%份额的投票权或表决权或者以任何其他方式直接或间接控制该实体50%以上的具有该实体决策权的所有者权益。

    **1.2  遵从软件：** 是指由华为技术有限公司（后称“华为”）正式发布且未经过修改的UniProton，或者虽经修改，但能通过认证测试的UniProton。

    **1.3  认证测试：** 是指为了确保UniProton与其一起使用的软件或硬件的兼容性和接口一致性而由华为开发的测试。认证测试套件及相关要求和指导将在UniProton官网公布。

    **1.4  贡献：** 是指由任何人提交给UniProton将其纳入或建议纳入UniProton中的任何信息或资料，包括软件源代码，文档或相关材料。

    **1.5  贡献者：** 是指提交贡献给UniProton的人。

    **1.6  您：** 是指任何个人，公司，合作企业，共同控股公司，有限合伙，协会，有限责任公司或企业或实体。

    **1.7  UniProton：** 是指由华为主导开发、管理、并由华为在openEuler官网上发布的可用于芯片，嵌入式领域的轻量开源操作系统项目。华为可能会对该软件不断进行更新。

    **1.8  承诺的专利权利要求：** 是指专利或专利申请的一个或多个权利要求，且满足如下条件：（i）由贡献者或其关联公司现在或将来拥有控制的，或其他有权许可且无需向无关联的第三方支付费用的，且（ii）会因接受者制造，使用，许诺销售，销售，进口或以其他方式转移贡献者提交给UniProton的贡献单独或将该贡献与前述UniProton的结合所必然且直接侵犯。

    **1.9  政策：** 是指本UniProton知识产权政策。

    **1.10   接收者：** 是指接受本政策并享有本政策下许可的个人或法律实体。

    **1.11  承诺者：** 是指贡献者及其关联公司。

2.  许可

    UniProton的代码将以MulanPSL2 License，除非华为另选其他许可证 （“可适用的许可证”）。接收者可以访问  [http://license.coscl.org.cn/MulanPSL2/index.html](http://license.coscl.org.cn/MulanPSL2/index.html)查看该许可证的详细内容。

3.  专利不诉承诺

    3.1 在接收者遵守本政策的前提下，每个承诺者许诺不对任何接收者就其制造，使用，许诺销售，销售，进口或以其他方式转移遵从软件的行为发起指控，诉讼或其他法律程序指控其侵犯该承诺者的承诺专利权利主张。前述承诺不适用于因以下原因引起的侵犯承诺专利权利主张的指控：i. 由其他方提交的贡献；ii 承诺者贡献代码后其他人对该贡献的修改；iii 该贡献与硬件的结合或者与不属于贡献所针对的UniProton的其他代码的结合；或者 iv 不属于遵从软件的软件。前述承诺也不适用于集成在个人便携产品（如手机，便携电脑，可穿戴设备等）中的遵从软件。

    3.2 这是每个承诺者对接收者的个人承诺。

    3.3 每个承诺者理解并同意专利不诉承诺是具有法律约束力且不可撤回的（根据第4条撤回的除外），而且对该承诺者、其继受者、受让方以及有权对第三方实施该承诺专利主张的任何独占被许可人都是有约束力的。

4.  终止许可及专利不诉承诺的权利

    在满足可适用许可证规定的前提下，如果贡献者或其关联公司向其他基于第3条享有专利不诉承诺权益且未根据本条终止的接收者发起指控，诉讼或其他法律程序，指控其制造，使用，许诺销售，销售，进口或者以其他形式转移遵从软件构成对其专利直接侵权或帮助侵权的，那么该接收者及其关联公司本政策下所有的许可以及专利不诉承诺将立即终止。

5.  无其他权利

    5.1 本政策并不授予接收者使用华为的贸易名称，商标，服务标记或产品名称的许可。

    5.2 除本政策和可使用许可证明确约定外，没有其他明示或默示的专利，商标，版权或其他知识产权的许可授予接收者，不论是通过默示，放弃，禁止反言或其他形式。

6.  无担保

    除非适用法强制规定或者双方有明确书面约定, UniProton, 遵从软件以及任何贡献均如是提供，无任何形式的担保，不论是明示还是默示，包括但不限于不侵权，适销性或满足特定目的的担保。

7.  责任限制

    除非适用法强制规定，在任何情况下，不论是基于侵权，合同或其他，华为及其关联公司或任何贡献者均不对其他贡献者，接收者或第三方因本政策或因使用或未能使用华为发布的UniProton或者对UniProton的任何贡献导致的损失，包括任何直接的，间接的，特别的，偶然的损失或者数据丢失等，即使该方已经被建议该种可能的损失。

8.  可分离性

    如果本政策内的任何条款被认定为无效，不可实施或者与适用法冲突，本政策中的其他条款继续有效。

9.  修改

    华为有权自行决定修改本政策。该修改后的政策从其在UniProton官网上公布之日起生效，并仅适用于同时或此后发布的UniProton软件版本。

10. 适用法和争议解决

    本政策适用中华人民共和国法律。任何因本政策引起或与本政策相关的纠纷应提交到北京中国国际经济贸易仲裁委员会。该仲裁裁决是终局的并对仲裁双方具有法律约束力。

## 联系我们

* 技术支持

  欢迎<a href="https://gitee.com/openeuler/UniProton/issues" target="_blank">提交issue</a>对关心的问题发起讨论，欢迎到<a href="https://bbs.huaweicloud.com/" target="_blank">UniProton论坛</a>交流。
  您也可以发送问题至邮箱UniProtonSupport@huawei.com。

* 参与贡献

  如您有兴趣参与开源贡献，欢迎提交PR参与特性建设，可至<a href="https://gitee.com/openeuler/UniProton " target="_blank">UniProton gitee源码仓</a>下载开源代码。

* 技术合作

  如您有合作意向，希望加入UniProton生态合作伙伴，请发邮件至UniProtonSupport@huawei.com，或访问<a href="https://www.openeuler.org/zh/" target="_blank">openEuler官网</a>，进一步了解详细信息。
