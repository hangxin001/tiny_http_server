#include"httpRequest.h"
HttpRequest::HttpRequest(int fd) : fd_(fd), working_(false), timer_(nullptr), version_(unKnown),
method_(Invaild), parseStatus_(RequestLine) {};

HttpRequest::~HttpRequest(){
	close(fd_);
}

int HttpRequest::recv(int* savedError) {
	return inputBuffer_.readFd(fd_,savedError);
}
int HttpRequest::send(int* savedError) {
	return outputBuffer_.writeFd(fd_,savedError);
}
void HttpRequest::appendReponse(const Buffer& buffer) {
	outputBuffer_.append(buffer);
}
void HttpRequest::appendInBuffer(const std::string& str) {
	inputBuffer_.append(str);
}
bool HttpRequest::parseRequestLine() {
	const char* SPACE = " ";
	const char* crlf = inputBuffer_.findCRLF();
	const char* firstSpace = std::search(inputBuffer_.peek(), crlf, SPACE,SPACE+1);
	const char* secondSpace = std::search(firstSpace + 1, crlf, SPACE, SPACE+1);
	if (firstSpace == crlf || secondSpace == crlf) {	//http�����б��������ո�ָ�
		method_ = Invaild;
		return false;
	}
	if (std::equal(inputBuffer_.peek(), firstSpace - 1, "POST")) {
		method_ = Post;
	}
	else if (std::equal(inputBuffer_.peek(), firstSpace - 1, "GET")) {
		method_ = Get;
	}
	else if (std::equal(inputBuffer_.peek(), firstSpace - 1, "HEAD")) {
		method_ = Head;
	}
	else
		method_ = Invaild;

	path_ = std::string(firstSpace + 1, secondSpace);	//����url

	if (std::equal(secondSpace + 1, crlf, "HTTP/1.1"))	//����httpVersion
		version_ = HTTP11;
	else if (std::equal(secondSpace + 1, crlf, "HTTP/1.0"))
		version_ = HTTP10;
	else
		version_ = unKnown;
	inputBuffer_.retrieve(crlf - inputBuffer_.peek() + 2);
	parseStatus_ = RequestHead;
	return true;
}
bool HttpRequest::parseRequestHead() {		//����std::string������������Ŀ��ɺ���һ�Դ���string��Ч�����
	const char* COLON = ":";
	while (1) {			
		const char* crlf = inputBuffer_.findCRLF();
		if ( crlf == nullptr) {		//ֱ���Ҳ���crlf�������
			break;
		}
		const char* colon = std::search(inputBuffer_.peek(), crlf, COLON, COLON + 1);
		if (colon != crlf) {
			headers_.insert(std::pair<std::string, std::string>
				(std::string(inputBuffer_.peek(),colon), std::string(colon+1,crlf)));
		}
		inputBuffer_.retrieve(crlf - inputBuffer_.peek() + 2);
	}
	parseStatus_ = RequestBody;
	return true;
}
bool HttpRequest::parseRequestQuery() {
	if (method_ == Get || method_ == Head) {		//����Get��query
		std::string::size_type quest = path_.find('?');	
		if (quest != path_.npos) {
			query_.assign(path_, quest + 1, path_.length() - 1);
			path_.assign(path_, 0, quest);
		}
	}
	if (method_ == Post) {
		if (inputBuffer_.readableBytes() > 0) {
			query_ = std::string(inputBuffer_.peek(), inputBuffer_.readableBytes());
		}
	}
	parseStatus_ = Finish;
	return true;
}
bool HttpRequest::parseRequest() {
	parseRequestLine();
	parseRequestHead();
	parseRequestQuery();
}
void HttpRequest::resetParse() {
	parseStatus_ = RequestLine;
	method_ = Invaild;
	version_ = unKnown;
	headers_.clear();
	path_ = "";
	query_ = "";

}
bool HttpRequest::keepAlive() {
	auto alive = headers_.find("Connection");
	std::string str;
	if (alive != headers_.end()) {
		str = alive->second;
	}
	bool test = (method_ == HTTP11);
	if ( str == "Keep-Alive" || (version_ == HTTP11 && str != "close"))
		return true;
	return false;
}
