#include"buffer.h"
#include<error.h>
#include<sys/socket.h>
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
    ssize_t nowWrite = 0;
    ssize_t totalWrite = readableBytes();
    while(nowWrite <totalWrite){
        n = send(fd,begin()+readIndex_,readableBytes(),0);
        if(n == -1 && errno != EAGAIN)
            break;
        readIndex_ += static_cast<size_t>(n);
        nowWrite +=n;
    }

    if(n < 0 && n == EINTR){
        return 0;
    }
    else if(n < 0){
        perror("Buffer write error:");
        *savedError = errno;
    }
    return n;
}  
