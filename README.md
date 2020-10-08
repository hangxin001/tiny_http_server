
# tiny_http_server
a tiny http server
一个简单的http服务器，使用C++开发。使用了C++11新特性。
因为封装的是epoll，所以仅能在Linux下使用。

特性：
 - Reactor模型
 - 基于epoll的多路复用
 - 支持Get,Post,Head三种请求方法，同时支持浏览缓存（IMS头）
 - 可自动增长的缓冲区
 - 基于priority_queue与shared_prt的应用层定时器

NOW TESTING
- [X] Epoll

- [X] Thread Pool

- [X] Http Server

- [X] Http Request

- [X] Http Respone

- [X] Buffer

- [X] Timer

Class
- Epoll:封装epoll
- Thread Pool:轻量线程池
- Http Server:管理线程池，定时器，并分配任务
- Http Request:解析http请求报文
- Http Respone:响应http请求
- Buffer:缓存区类（参考了muduo的buffer）
- Timer:基于优先队列的应用层定时器