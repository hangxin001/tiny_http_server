#pragma once

class HttpRequest {
public:

	int getFd(){
		return fd_;
	}
private:
	int fd_;
};