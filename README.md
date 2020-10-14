
# tiny_http_server
a tiny http server
一个简单的http服务器，使用C++开发。使用了C++11新特性。

默认目录为二进制文件下的./www

特性：
 - Reactor模型
 - 基于epoll的多路复用
 - 支持Get,Post,Head三种请求方法，同时支持浏览缓存（IMS头）
 - 可自动增长的缓冲区
 - 基于priority_queue与shared_prt的应用层定时器


Class
- Epoll:封装epoll
- Thread Pool:轻量线程池
- Http Server:管理线程池，定时器，并分配任务
- Http Request:解析http请求报文
- Http Response:响应http请求
- Buffer:缓存区类（参考了muduo的buffer）
- Timer:基于优先队列的应用层定时器



硬件环境：
- I7 9400f
- RAM 16G

软件环境
- gentoo
- g++ 10.2
- LinuxKernel 5.8

Bug
- Not Modified报文，TTFB时间无论什么情况都会固定在500ms。
<br>
原因：chrome计算TTFB时间方式的不同。首先服务器设定500ms是主动断开时间。同过抓包就能发现回应报文时间正常。但chrome收到服务端的Not Modified(304)报文后并没有结束计时。而是继续记时。直到tcp连接关闭，由于服务端设置500ms后超时主动关闭连接，所以TTFB时间会固定在500ms。但是在get的情况不下就不一样，chrome不会等待到tcp流的关闭，而是只要收到相应的数据就停止计时。