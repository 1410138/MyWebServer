# MyWebServer
C++ Linux Web服务器

## 实现
- 使用**Reactor**+**非阻塞socket**+**epoll(ET)**+**线程池**实现多线程服务器模型
- 实现了基于**最小堆**的定时器，关闭超时未活动的连接
- 使用**function**+**bind**的C++11&14新语法代替虚函数实现回调
- 使用**vector容器**实现自动增长的用户缓冲区buffer
- 使用**单例模式**与**阻塞队列**实现了同步+异步的日志系统
- 实现了**数据库连接池**，减少数据库连接建立与关闭的开销，提高访问效率
- 使用**shared_ptr**与**unique_ptr**完成RAII手法，实现对资源的自动管理
- 使用**正则表达式**与**状态机**解析HTPP请求报文，支持解析**GET**与**POST**请求
- 使用**mmap内存映射**方法，提高服务器从磁盘中读取响应文件的速度
- 开启**SO_LINGER**套接字选项，实现close函数的延时返回，确保对端已确认全部数据

## 原理图
[![Reactor11e8ae0507415d5a.md.png](https://img.picgo.net/2023/01/12/Reactor11e8ae0507415d5a.md.png)](https://www.picgo.net/image/857Ij)

## 测试环境
- CentOS 7
- MySQL 5.7
- Firefox

## 运行
```
// 建立yourdb库
create database yourdb;

// 创建user表
USE yourdb;
CREATE TABLE user(
    username char(50) NULL,
    password char(50) NULL
)ENGINE=InnoDB;

// 添加数据
INSERT INTO user(username, password) VALUES('name', 'password');
```
```
make
./myWebServer
```
打开浏览器，网址栏输入http://本机IP:8888 ，进入Web服务器主页

## 运行结果
[![e8862dcae57c337514ca7440db1c5bd331a497b09f8bac5e.jpeg](https://img.picgo.net/2023/01/12/e8862dcae57c337514ca7440db1c5bd331a497b09f8bac5e.jpeg)](https://www.picgo.net/image/%E4%B8%BB%E9%A1%B5.8wdJC)

如图，拥有展示主页、获取图片与视频以及注册和登录的功能

## 压力测试
[![22ce1cd158fc233d4f63edf21a963fac5353692dc022aa35.md.jpeg](https://img.picgo.net/2023/01/17/22ce1cd158fc233d4f63edf21a963fac5353692dc022aa35.md.jpeg)](https://www.picgo.net/image/%E5%8E%8B%E5%8A%9B%E6%B5%8B%E8%AF%95.dGhIA)

1000个客户端并发访问网站30s，每分钟响应请求数：183350 pages/min ,每秒钟传输数据量 1824332 bytes/sec，并发1000运行30秒后产生的TCP连接91675个，0 failed.

## 实现细节
- [基于C++11实现的阻塞队列(BlockQueue)](https://blog.csdn.net/weixin_50437588/article/details/128434180?spm=1001.2014.3001.5502)
- [基于最小堆实现的定时器（HeapTimer）](https://blog.csdn.net/weixin_50437588/article/details/128435637?spm=1001.2014.3001.5502)
- [基于C++11的线程池实现（ThreadPool）](https://blog.csdn.net/weixin_50437588/article/details/128449258?spm=1001.2014.3001.5502)
- [缓冲区Buffer类的设计（参考Muduo实现）](https://blog.csdn.net/weixin_50437588/article/details/128488095?spm=1001.2014.3001.5502)
- [同步+异步日志系统（C++实现）](https://blog.csdn.net/weixin_50437588/article/details/128511229?spm=1001.2014.3001.5502)
- [数据库连接池（C++11实现）](https://blog.csdn.net/weixin_50437588/article/details/128513581?spm=1001.2014.3001.5502)
- [解析HTTP请求报文（GET、POST）](https://blog.csdn.net/weixin_50437588/article/details/128570178?spm=1001.2014.3001.5502)
- [生成HTTP响应报文](https://blog.csdn.net/weixin_50437588/article/details/128663171?spm=1001.2014.3001.5501)
- [HTTP连接（读取请求+解析请求+生成响应+回送响应）](https://blog.csdn.net/weixin_50437588/article/details/128664609?spm=1001.2014.3001.5501)
- [Linux多线程Web服务器（C++实现）](https://blog.csdn.net/weixin_50437588/article/details/128667510)
