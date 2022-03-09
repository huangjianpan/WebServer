# WebServer

## 简介

一个轻量的Web服务器，使用TCP协议。

* 多Reactor多线程，其中主Reactor线程接收连接，从Reactor线程负责IO

* 使用epoll + 非阻塞IO + 水平触发模式

* 支持HTTP长连接，支持GET方法

* 使用红黑树管理定时器

* 使用智能指针管理对象生命周期



## 开发环境

* 操作系统：Ubuntu 18.04
* 编译器：g++ 8.4.0
* 自动化构建：CMake
* 编辑器：Visual Studio Code
* 测压工具：WebBench 1.5