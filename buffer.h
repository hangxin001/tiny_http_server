#pragma once
#include<iostream>
#include<string>
#include<vector>
#include<algorithm>
#include<assert.h>
#include<sys/uio.h>
#include<unistd.h>
/// 参考了muduo的buffer设计
/// www.cnblogs.com/solstice/archive/2011/04/17/2018801.htmls
/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode
class Buffer {
public:
	static const size_t CHEAP_PREPEND = 8;   //为prepend预留空间
	static const size_t INIT_SIZE = 1024;

	Buffer() :buffer_(CHEAP_PREPEND + INIT_SIZE), readIndex_(CHEAP_PREPEND), writerIndex_(CHEAP_PREPEND) {};
	~Buffer() {};

	ssize_t readFd(int fd, int* savedError);  //将数据从fd移到buffer
	ssize_t writeFd(int fd, int* savedError);  //将buffer数据发送给fd


	size_t readableBytes() const {		//可读空间数量
		return writerIndex_ - readIndex_;
	}
	size_t writrableBytes() const {		//可写空间数量
		size_t t = buffer_.size() - static_cast<size_t>(writerIndex_);
		return t;
	}
	size_t prependableBytes() const {		//很棒的想法，可惜这个项目用不上
		return readIndex_;
	}

	void swap(Buffer& rhs) {
		buffer_.swap(rhs.buffer_);
		std::swap(readIndex_, rhs.readIndex_);
		std::swap(writerIndex_, rhs.writerIndex_);
	}
	const char* peek() const {		//返回第一个可读指针
		return begin() + readIndex_;
	}
	char * beginWrite(){
		return begin() + writerIndex_;
	}
	const char* beginWrite() const {
		return begin() + writerIndex_;
	}
	
	const char* findCRLF() const{
		const char CRLF[] = "\r\n";
		const char* crlf = std::search(peek(), beginWrite(), CRLF, CRLF + 2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	const char* findCRLF(const char* start) const {		//寻找下一个CRLF
		assert(start < begin());  //越界
		const char CRLF[] = "\r\n";
		const char* crlf = std::search(start, beginWrite(), CRLF, CRLF + 2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	std::string retrieveUntilCRLFAsString() {		//取出到下一个CRLF之前的数据，并丢弃CRLF，string性能拉跨，废弃
		const char* CRLF = findCRLF();
		if (CRLF == nullptr)
			return std::string();
		else if (peek() == CRLF) {
			retrieve(CRLF + 2 - peek());
			return std::string();
		}
		std::string str(peek(), CRLF - 1);
		retrieve(CRLF + 2 - peek());
		return str;
	}
	void retrieve(size_t length){		//readIndex后移,取出length长度
		if(readableBytes() >= length)
			readIndex_ += length;
		else
			retrieveAll();			//超过则直接取出全部
		
	}
	void retrieveAll(){
		readIndex_ = CHEAP_PREPEND;
		writerIndex_ = CHEAP_PREPEND;
	}
	std::string retrieveAsString(size_t length) {
		if (readIndex_ + length > writerIndex_)
			return std::string();		//错误即返回空string
		std::string str(peek(), peek() + length);
		retrieve(length);
		return str;
	}
	std::string retrieveAllAsString(){
		std::string str(peek(),readableBytes()); //使用构造string (const char* s, size_t n);
		retrieveAll();
		return str;
	}
	void hasWrite(size_t length){
		writerIndex_ += length;
	}
	void ensureWriterableBytes(size_t length){
		if(length > writrableBytes())
			makeSpace(length);
	}
	void append(const char* str, size_t length){
		ensureWriterableBytes(length);
		std::copy(str,str+length,beginWrite());
		hasWrite(length);
	}
	void append(std::string str){
		append(str.data(),str.length());	//data() 返回的指针则保证指向一个size()长度的空间
	}
	void append(Buffer other){
		append(other.peek(),other.readableBytes());
	}


private:
	char* begin() {
		return &*buffer_.begin();
	}
	const char* begin() const {
		return &*buffer_.begin();
	}
	void makeSpace(size_t length) {
		if (writrableBytes() + prependableBytes() < length + CHEAP_PREPEND) {
			buffer_.resize(length + writerIndex_);
		}
		else {  //将以读过的数据覆盖。
			size_t readable = readableBytes();
			std::copy(begin()+readIndex_, begin()+ writerIndex_, begin() + CHEAP_PREPEND);
			readIndex_ = CHEAP_PREPEND;
			writerIndex_ = readIndex_ + readable;

		}

	}
private:
	std::vector<char> buffer_;
	size_t readIndex_;
	size_t writerIndex_;
};
