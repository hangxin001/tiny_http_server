#pragma once
#include"threadPool.h"
#include"timer.h"
#include"epoll.h"
#include"httpRequest.h"
#include"httpResponse.h"
#include"buffer.h"
#include<memory>
#include<chrono>
class HttpServer{
public:
    const int MAXLISTEN = 4096;
    const int TIMEOUT = 500;
    HttpServer();   //默认127.0.0.1:3333 6
    HttpServer(int port, int worker);   //默认127.0.0.1
    HttpServer(std::string ip, int port, int worker);
    ~HttpServer();
    void run();

private:
    void newConnetion();    
    void closeConnetion(HttpRequest* httpRequest);  //与Epoll内对应
    void handleRequest(HttpRequest* httpRequest);
    void handleResponse(HttpRequest* httpRequest);
    int createServerFd(std::string ip, int port);
    bool setNoBloking(int fd);

private:
    using HttpRequestPtr = std::shared_ptr<HttpRequest>;
    using EpollPtr = std::shared_ptr<Epoll>;
    using ThreadPoolPtr = std::shared_ptr<ThreadPool>;
    using TimerManagerPtr = std::shared_ptr<TimerManager>;

    int serverFd_;
    int port_;   //默认port33333
    HttpRequestPtr serverRequest_; //封装serverFd的HtppRequest
    EpollPtr epoll_;
    TimerManagerPtr timerManager_;
    ThreadPoolPtr threadpool_;
};

