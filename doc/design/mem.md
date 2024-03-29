# 内存管理介绍

内存管理主要工作是动态的划分并管理用户分配好的内存区间。当程序某一部分需要使用内存，可以通过操作系统的内存申请函数索取指定大小内存块，一旦使用完毕，通过内存释放函数归还所占用内存，使之可以重复使用。在系统运行过程中，内存管理模块通过对内存的申请/释放操作管理用户和OS对内存的使用，使内存的利用率和使用效率达到最优。

## 内存基本概念

### 内存块 slice
用户申请到的一片连续内存空间，是内存管理的最小单元。

### 算法 arithmetic
内存管理的一种策略，如FSC。

## 内存算法

目前提供了私有FSC算法

## 开发流程
### 步骤一：设置内存管理模块配置项

使用UniProton内存管理模块，需要进行配置项的设置，需要配置的项包括缺省分区首地址、分区大小等。

### 步骤二：使用内存管理模块

当需要使用内存时，需要先创建一个指定内存管理算法的内存分区，通过调用内存申请接口申请合适大小的内存，就可以对申请到的内存进行操作（包括写操作，然后给其他模块传递消息等）；如果是动态内存，当内存使用完，需要对这块内存进行释放，防止发生内存泄漏。
