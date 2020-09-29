#pragma once
#include<iostream>
#include<string>
#include<vector>
#include <algorithm>
#include<assert.h>
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
	
	size_t readableBytes() const {		//可读空间数量
		return writerIndex_ - readIndex_;
	}
	size_t writrableBytes() const {		//可写空间数量
		return buffer_.size() - writerIndex_;
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
	const char* beginWrite() const {
		return begin() + writerIndex_;
	}
	
	const char* findCRLF() const{
		const char CRLF[] = "\n\r";
		const char* crlf = std::search(peek(), beginWrite(), CRLF, CRLF + 2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	const char* findCRLF(const char* start) const {
		assert(start < begin());  //越界
		const char CRLF[] = "\n\r";
		const char* crlf = std::search(start, beginWrite(), CRLF, CRLF + 2);
		return crlf == beginWrite() ? nullptr : crlf;
	}
	void append();

private:
	char* begin() {
		return &*buffer_.begin();
	}
	const char* begin() const {
		return &*buffer_.begin();
	}
	void makeSpace(size_t length) {
		if (writrableBytes() + prependableBytes() < length + CHEAP_PREPEND) {
			buffer_.resize(length + writrableBytes());
		}
		else {  //将以读过的数据覆盖。
			size_t readable = readableBytes();
			std::copy(peek(), beginWrite(), begin() + CHEAP_PREPEND);
			readIndex_ = CHEAP_PREPEND;
			writerIndex_ = readIndex_ + readable;

		}

	}
private:
	std::vector<char> buffer_;
	size_t readIndex_;
	size_t writerIndex_;
};