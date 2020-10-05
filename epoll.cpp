#include"epoll.h"

Epoll::Epoll() :epollFd_(epoll_create1(EPOLL_CLOEXEC)),eventList_(MAXCONNECT) {};

Epoll::~Epoll() {
	close(epollFd_);
}
int Epoll::add(int fd, int events, void* ptr) {
	epoll_event event;
	event.events = events;
	event.data.ptr = ptr;
	int ret = epoll_ctl(epollFd_, EPOLL_CTL_ADD, fd, &event);
	return ret;
}
int Epoll::mod(int fd, int events, void* ptr) {
	epoll_event event;
	event.events = events;
	event.data.ptr = ptr;
	int ret = epoll_ctl(epollFd_, EPOLL_CTL_MOD, fd, &event);
	return ret;
}
int Epoll::del(int fd, int events, void* ptr) {
	epoll_event event;
	event.events = events;
	event.data.ptr = ptr;
	int ret = epoll_ctl(epollFd_, EPOLL_CTL_DEL, fd, &event);
	return ret;
}
int Epoll::wait(int timeOut) {
	int ret = epoll_wait(epollFd_, &*(eventList_.begin()), static_cast<int>(eventList_.size()), timeOut);
	if (ret < 0)
		perror("epoll wait:");
	return ret;
}
void Epoll::setNewConnectCallback(const NewConnetCallback& fun) {
	newConnetCallback_ = fun;
}
void Epoll::setCloseConnetCallback(const CloseConnetCallback& fun) {
	closeConnetCallback_ = fun;
}
void Epoll::setHandleInCallback(const HandleInCallback& fun) {
	epollinCallback_ = fun;
}
void Epoll::setHandleOutCallback(const HandleOutCallback& fun) {
	epolloutCallback_ = fun;
}
void Epoll::eventTrick(int serverFd, ThreadPool& T, int eventsSum) {
	for (int i = 0; i < eventsSum; ++i) {
		HttpRequest* reRequest = static_cast<HttpRequest*>(eventList_[i].data.ptr);

		if (reRequest->getFd() == serverFd)  //ÐÂÁ¬½Ó
			newConnetCallback_();
		else if (
			(eventList_[i].events & EPOLLHUP) ||
			(eventList_[i].events & EPOLLERR)){
			closeConnetCallback_(reRequest);
		}
		else if (eventList_[i].events & EPOLLIN) {
			epollinCallback_(reRequest);
		}
		else if (eventList_[i].events & EPOLLOUT) {
			epolloutCallback_(reRequest);
		}
		else {
			//debug
			std::cout << "EventTrick: fd "<< reRequest->getFd() << " bad events" << std::endl;
			;
		}
	}
}