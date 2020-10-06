
# tiny_http_server
a tiny http server
一个简单的http服务器，使用C++开发。使用了C++11新特性。
因为封装的是epoll，所以仅能在Linux下使用。

特性：
 - Reactor模型
 - 多路复用
 - 支持HTTP长连接
 - 可自动增长的缓冲区（参考了muduo的buffer）
 - 基于priority_queue与shared_prt的应用层定时器

NOW CODING
- [X] Epoll

- [X] Thread Pool

- [ ] Http Server

- [X] Http Request

- [ ] Http Respone

- [X] Buffer

- [X] Timer