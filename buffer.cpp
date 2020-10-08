#include"buffer.h"
#include<error.h>

ssize_t Buffer::readFd(int fd , int* savedError){   //配合Epoll的LT模式
    char extrabuff[65536];
    iovec iov[2];
    const size_t writable = writrableBytes();
    iov[0].iov_base = begin() + writerIndex_;
    iov[0].iov_len = writable;
    iov[1].iov_base = extrabuff;
    iov[1].iov_len = sizeof(extrabuff);
    const ssize_t n = readv(fd,iov,2);  //巧用栈上空间
    if( n < 0){  //error
        perror("Buffer readv error:");
        *savedError = errno;
    }
    else if ( n < writable)
    {
        hasWrite(static_cast<size_t>(n));
    }
    else{
        writerIndex_ = buffer_.size();
        append(extrabuff,n-writable);
    }
    return n;

}
ssize_t Buffer::writeFd(int fd, int* savedError){
    ssize_t n;
    n = write(fd,begin()+readIndex_,readableBytes());
    if(n < 0 && n == EINTR){
        return 0;
    }
    else if(n < 0){
        perror("Buffer write error:");
        *savedError = errno;
    }
    else{
        readIndex_ += static_cast<size_t>(n);
        return n;
    }
}  
/*
ssize_t Buffer::readFd(int fd , int* savedError){   //如果读就要全部读完，配合Epoll的ET模式
    ssize_t n = 0;
    char extrabuff[65536];
    iovec iov[2];
    while(1){  //巧用栈上空间
        iovec iov[2];
        const size_t writable = writrableBytes();
        iov[0].iov_base = begin() + writerIndex_;
        iov[0].iov_len = writable;
        iov[1].iov_base = extrabuff;
        iov[1].iov_len = sizeof(extrabuff);
        n = readv(fd,iov,2);
        if(n <= 0)
            break;
        
        if ( n < writable)
        {
            hasWrite(static_cast<size_t>(n));
        }
        else{
            writerIndex_ = buffer_.size();
            append(extrabuff,n-writable);
        }
    }
    if(n == -1 && errno != EAGAIN){
        perror("buffer read");
        *savedError = errno;
        return -1;
    }
 
    return 1;

}
ssize_t Buffer::writeFd(int fd, int* savedError){
    ssize_t n = 0;
    ssize_t writeN = 0;     //以写的量
    ssize_t nsize = readableBytes();    //要写的总量
    while( writeN <= nsize){
        n = write(fd,begin()+readIndex_,readableBytes());
        if(n <= 0) //错误跳出
            break;
        writeN  += n;
        readIndex_ += static_cast<size_t>(n);
    }
    if(n == -1 && errno != EAGAIN){
        perror("buffer write");
        *savedError = errno;
        return -1;
    }
    return 1;
    
}*/