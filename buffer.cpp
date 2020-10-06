#include"buffer.h"
#include<error.h>
ssize_t Buffer::readFd(int fd , int* savedError){
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