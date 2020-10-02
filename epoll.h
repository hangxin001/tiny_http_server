#pragma once
#include<sys/epoll.h>
#include<functional>
#include<vector>
class Epoll {
public:
	static const int DEFAULCONNET = 1000;
	Epoll();
	~Epoll();
	using NewConnetCallback = std::function<void()>;
	using CloseConnetCallback = std::function<void()>;
	using HandleInCallback = std::function<void()>;		//����EPOLLIN
	using HandleOutCallback = std::function<void()>;	//����EPOLLOUT
	int add(int fd, int events, void* ptr);
	int del(int fd, int events, void* ptr);
	int mod(int fd, int events, void* ptr);
	int wait(int timeOut);	//��λΪMS
	int eventTrick();
	void setNewConnectCallback(const NewConnetCallback& fun);
	void setCloseConnetCallback(const CloseConnetCallback& fun);
	void setHandleInCallback(const HandleInCallback& fun);
	void setHandleOutCallback(const HandleOutCallback& fun);

private:
	int epoll_fd_;
	std::vector<epoll_event> eventList_;

	NewConnetCallback newConnetCallback_;
	CloseConnetCallback closeConnetCallback_;
	HandleInCallback epollinCallback_;
	HandleOutCallback epolloutCallback;


};