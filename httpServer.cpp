#include"httpServer.h"
#include<fcntl.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<cstring>
bool HttpServer::setNoBloking(int fd){
    int flag = fcntl(fd, F_GETFD, 0);
    if(flag == -1){
        perror("HttpSever setNoBloking:");
        return false;
    }
    flag |= O_NONBLOCK;
    if(fcntl(fd,F_SETFD,flag) == -1){
        perror("HttpSever setNoBloking:");
        return false;
    }
    return true;
}
int HttpServer::createServerFd(std::string ip, int port){
    port <= 0 || port > 65535 ? port = 33333 : port;
    int serverFd;

    struct sockaddr_in serverAddr;  //设置地址
    bzero(&serverAddr.sin_zero,sizeof(serverAddr.sin_zero));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = inet_addr(ip.data());
    serverAddr.sin_port = htons(static_cast<uint16_t>(port));

    if((serverFd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1){
        perror("create socket:");
        close(serverFd);
        return -1;
    }
    int optval = 1;     //解决Address already in use
    if(setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1) {
        perror("creat socket:");
        close(serverFd);
        return -1;
    }
    if((bind(serverFd, (struct sockaddr*)&serverAddr, sizeof(serverAddr))) == -1) {
        perror("creat socket:");
        close(serverFd);
        return -1;
    }
     if(listen(serverFd, MAXLISTEN) == -1) {
        perror("creat socket:");
        close(serverFd);
        return -1;
    }
    return serverFd;
}
HttpServer::HttpServer(std::string ip , int port, int worker):
    serverFd_(createServerFd(ip,port)),
    port_(port),
    serverRequest_(std::make_shared<HttpRequest>(serverFd_)),   //使用make_shared减少内存分配
    epoll_(std::make_shared<Epoll>()),
    timerManager_(std::make_shared<TimerManager>()),
    threadpool_(std::make_shared<ThreadPool>(worker))
{}

HttpServer::~HttpServer(){
    close(serverFd_);
}
void HttpServer::newConnetion(){
    //while为了处理ET模式下只通知一次。https://blog.csdn.net/eyucham/article/details/86502117
    while(1) {      
        int acceptFd = ::accept4(serverFd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if(acceptFd == -1) {
            if(errno == EAGAIN)
                break;
            perror("accept:");
            break;
        }
        HttpRequest* request = new HttpRequest(acceptFd);   //HttpRequest在这里new，在close函数中delete。
        std::shared_ptr<Timer> requestTimer = timerManager_->addTimer(TIMEOUT,std::bind(&HttpServer::closeConnetion,this,request));
        request->setTimer(requestTimer);    //设置定时器
        // 注册连接套接字到epoll
        epoll_ -> add(acceptFd, (EPOLLIN | EPOLLONESHOT), request);
        std::cout << "new connection"<< std::endl;
        openFd_++;
    }
}
void HttpServer::closeConnetion(HttpRequest* httpRequest){
    std::cout << "close connection"<< std::endl;
    int closeFd = httpRequest->getFd();
    if(httpRequest->isWorking()){    //在读写不可被关闭
        timerManager_->delTimer(httpRequest->getTimer());   
        return;
    }
    timerManager_->delTimer(httpRequest->getTimer());
    epoll_->del(closeFd,0,httpRequest);
    delete httpRequest; //HttpRequest析构会close（fd）
    httpRequest = nullptr;
    //debug
    closeFd_++;
    
}
void HttpServer::handleRequest(HttpRequest* httpRequest){   //LT
    timerManager_->delTimer(httpRequest->getTimer());
    int errorCode;
    int nRead = httpRequest->recv(&errorCode);

    if(nRead == 0 || (nRead <0 && errorCode != EAGAIN)){ //断开
        httpRequest->setWorking(false);
        closeConnetion(httpRequest);
        return;
    }

    httpRequest->parseRequest();    //解析报文，然后将返回信息写入缓冲区
    HttpResponse httpResponse(200,httpRequest->getPath(),httpRequest->getMethod(),httpRequest->getHeaders(),httpRequest->keepAlive());
    httpRequest->appendReponse(httpResponse.makeResponse());
    epoll_->mod(httpRequest->getFd(),(EPOLLIN | EPOLLOUT | EPOLLONESHOT),httpRequest);
}
void HttpServer::handleResponse(HttpRequest* httpRequest){
    timerManager_->delTimer(httpRequest->getTimer());
    int errorCode;
    int nWrite = httpRequest->send(&errorCode);

    if(nWrite == -1 && errorCode != EAGAIN){
        httpRequest->setWorking(false);
        closeConnetion(httpRequest);
        return;
    }
    if(!httpRequest->keepAlive()){
        closeConnetion(httpRequest);
        return;
    }
    std::shared_ptr<Timer> requestTimer = timerManager_->addTimer(TIMEOUT,std::bind(&HttpServer::closeConnetion,this,httpRequest));
    httpRequest->setTimer(requestTimer);    //设置定时器
    epoll_ -> add(httpRequest->getFd(), (EPOLLIN | EPOLLONESHOT ), httpRequest);
    httpRequest->setWorking(false);
}
void HttpServer::run(){
    epoll_->add(serverFd_, (EPOLLIN | EPOLLET) ,serverRequest_.get());
    epoll_->setNewConnectCallback(std::bind(&HttpServer::newConnetion,this));
    epoll_->setCloseConnetCallback(std::bind(&HttpServer::closeConnetion,this,std::placeholders::_1));  //std::placeholders::_1暂位符
    epoll_->setHandleInCallback(std::bind(&HttpServer::handleRequest,this,std::placeholders::_1));
    epoll_->setHandleOutCallback(std::bind(&HttpServer::handleResponse,this,std::placeholders::_1));

    while(1){
        int time = timerManager_->getExpireTime();
        int events = epoll_->wait(time);
        //perror("");
        std::cout << openFd_ <<" "<<closeFd_<<std::endl;
        if(events > 0){
            epoll_->eventTrick(serverFd_,threadpool_,events);
            
        }
        timerManager_->tick();
    }
}