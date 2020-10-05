#pragma once
#include<sys/epoll.h>
#include<functional>
#include<vector>
#include<unistd.h>
#include<iostream>
#include"threadpool.h"
#include"httpRequest.h"
class Epoll {
public:
	static const int MAXCONNECT = 4096;
	Epoll();
	~Epoll();
	using NewConnetCallback = std::function<void()>;
	using CloseConnetCallback = std::function<void(HttpRequest* httpRequest)>;
	using HandleInCallback = std::function<void(HttpRequest* httpRequest)>;		//处理EPOLLIN
	using HandleOutCallback = std::function<void(HttpRequest* httpRequest)>;	//处理EPOLLOUT
	int add(int fd, int events, void* ptr);
	int del(int fd, int events, void* ptr);
	int mod(int fd, int events, void* ptr);
	int wait(int timeOut);	//单位为MS
	void eventTrick(int fd,ThreadPool& T,int eventsSum);
	void setNewConnectCallback(const NewConnetCallback& fun);
	void setCloseConnetCallback(const CloseConnetCallback& fun);
	void setHandleInCallback(const HandleInCallback& fun);
	void setHandleOutCallback(const HandleOutCallback& fun);

private:
	int epollFd_;
	std::vector<epoll_event> eventList_;	//vector保证连续，不扩容的情况下指针不会失效

	NewConnetCallback newConnetCallback_;
	CloseConnetCallback closeConnetCallback_;
	HandleInCallback epollinCallback_;
	HandleOutCallback epolloutCallback_;


};