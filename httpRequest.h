#pragma once
#include"buffer.h"
#include"timer.h"
#include<unordered_map>
class HttpRequest {
public:
	enum RequestParseStatus{
		RequestLine,
		RequestHead,
		RequestBody,
		Finish
	};
	enum Method {
		Invaild,
		Get,
		Post,
		Head,
	};
	enum Version{
		unKnown,
		HTTP10,
		HTTP11
	};
	int revc();		//接收inBuffer数据
	int send();		//发送outBuffer数据
	void appendReponse(const Buffer& buffer);
	bool parseRequest();
	bool keepAlive();
	void resetParse();

	int getFd() {
		return fd_;
	}
	Method getMethod() const{
		return method_;
	}
	Version getVersion() const {
		return version_;
	}
	std::string getPath() const{
		return path_;
	}
	std::unordered_map<std::string, std::string> getHeaders() const {
		return headers_;
	}
	bool parseFinish() {
		return (parseStatus_ == Finish);
	}
	void setWorking(bool status) {
		working_ = status;
	}
	bool isWorking() const {
		return working_;
	}

	void setTimer(std::shared_ptr<Timer> timer) {
		timer_ = timer;
	}
	std::shared_ptr<Timer> getTimer() const {
		return timer_;
	}

private:
	int fd_;
	Buffer inputBuffer_;
	Buffer outputBuffer_;

	bool working_;
	std::shared_ptr<Timer> timer_;
	RequestParseStatus parseStatus_;
	Method method_;
	Version version_;
	std::string path_;	//请求路径
	std::string query_;	//暂时不打算处理动态请求
	std::unordered_map<std::string, std::string> headers_;

	bool parseRequestLine();
	bool parseRequestHead();
};