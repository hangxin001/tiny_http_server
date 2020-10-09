#include"buffer.h"
#include<error.h>
#include<sys/socket.h>
ssize_t Buffer::readFd(int fd , int* savedError){   //如果读就要全部读完，
    ssize_t n = 0;
    char extrabuff[65536];
    iovec iov[2];
    while(1){  //巧用栈上空间
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
            writerIndex_ = buffer_.size() ;
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
    ssize_t n;
    ssize_t nowWrite = 0;
    ssize_t totalWrite = readableBytes();
    while(nowWrite <totalWrite){
        n = write(fd,begin()+readIndex_,readableBytes());
        if(n == -1 && errno != EAGAIN)
            break;
        if( n<=0 && errno == EAGAIN)
            continue;
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
