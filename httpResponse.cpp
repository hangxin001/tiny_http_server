#include"httpResponse.h"
#include<fcntl.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/mman.h>
#include<time.h>
#include<iomanip> //put_time
#include<sstream>
const std::unordered_map<int, std::string> HttpResponse::status2Message ={
    {200, "OK"},
    {304, "Not Modified"},
    {400, "Bad Request"},
    {403, "Forbidden"},
    {404, "Not Found"},
    {500, "Internal Server Error"}
};
const std::unordered_map<std::string, std::string> HttpResponse::file2Type = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css"}
};

HttpResponse::HttpResponse(int statusCode,std::string path, std::string requestMethod, bool keepAlive)
:statusCode_(statusCode), path_(path), requestMethod_(requestMethod), keepAlive_(keepAlive){};

HttpResponse::HttpResponse(int statusCode,std::string path, std::string requestMethod,
std::unordered_map<std::string,std::string> requestHeader, bool keepAlive):
statusCode_(statusCode), path_(path), requestMethod_(requestMethod), requestHeader_(requestHeader), keepAlive_(keepAlive){};

std::string HttpResponse::getFileType(){
    int dot = path_.find_last_of('.');
    if(dot == path_.npos){   //找不到.
        return std::string("text/plain");
    }
    std::string type = path_.substr(dot);
    auto it = file2Type.find(type);
    if(it == file2Type.end()){
        return std::string("text/plain");
    }
    return it->second;
};

off_t HttpResponse::fileIsOK(std::string path){
    struct stat status;
    if(stat(path.data(),&status) < 0){
        statusCode_ = 404;
        return -1;
    }
    if(!(S_ISREG(status.st_mode) || !(S_IRUSR & status.st_mode))){
        statusCode_ = 403;
        return -1;
    }
    return status.st_size;
    
};
void HttpResponse::doNotModifiedResponse(Buffer& outBuffer){
    statusCode_ = 304;
    outBuffer.retrieveAll();
    appendResponseLine(outBuffer);
    outBuffer.append("Date:" + nowTime() + "\r\n");
    outBuffer.append("Server:Tiny_Http_Server\r\n");
}
void HttpResponse::appendResponseLine(Buffer& outBuffer){
    auto itr = status2Message.find(statusCode_);
    outBuffer.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + itr -> second + "\r\n"); 
}
void HttpResponse::appendResponseHeader(Buffer& outBuffer, off_t fileSize){     //迟早要重构，写的太难受了
    outBuffer.append("Date:" + nowTime() + "\r\n");
    outBuffer.append("Server:Tiny_Http_Server\r\n");
    //  304处理时间过长，需要排查
    /*if(!requestHeader_.empty()){    //处理If-Modified-Since
    
        auto it = requestHeader_.find("If-Modified-Since");
        if(it!=requestHeader_.end()){
            
            time_t lastTime;
            std::tm t_time ={};
            std::stringstream sstr(it->second);
            sstr >> std::get_time(&t_time,"%a, %d %b %Y %X GMT");
            lastTime = mktime(&t_time);
            struct stat stauts;
            stat(path_.data(),&stauts);
            time_t fileTime = stauts.st_mtime;
            if(fileTime == lastTime){
                doNotModifiedResponse(outBuffer);
                return;
            }
        }
    }*/
    
    {
    struct stat stauts;     //加入Last-Modified
    std::string sTime;
    stat(path_.data(),&stauts);
    time_t fileTime = stauts.st_mtime;
    std::stringstream sstr;
    sstr << std::put_time(std::localtime(&fileTime), "%a, %d %b %Y %X GMT");
    std::getline(sstr,sTime);
    outBuffer.append("Last-Modified:" + sTime + "\r\n");
    }

    if(keepAlive_) {
        outBuffer.append("Connection:Keep-Alive\r\n");
        outBuffer.append("Keep-Alive:timeout=" + std::to_string(CONNECT_TIMEOUT) + "\r\n");
    } else {
        outBuffer.append("Connection:close\r\n");
    }
    outBuffer.append("Content-type:" + getFileType() + "\r\n");
    outBuffer.append("Content-length:" + std::to_string(fileSize) + "\r\n");
    outBuffer.append("\r\n");
}
void HttpResponse::appendResponseBody(Buffer& outBuffer , off_t fileSize){  //默认文件为path_
    int fileFd = open(path_.data(),O_RDONLY,0);
    void *mmapAddr = mmap(NULL,fileSize,PROT_READ,MAP_PRIVATE,fileFd,0);
    close(fileFd);
    if( mmapAddr == (void*)-1){     //static_cast<void*>(-1) 无法转换
        munmap(mmapAddr,static_cast<size_t>(fileSize));
        outBuffer.retrieveAll();
        statusCode_ = 404;
        doErrorResponse(outBuffer,"Not Found");
        return;
    }
    outBuffer.append(static_cast<char*>(mmapAddr),static_cast<size_t>(fileSize));
    munmap(mmapAddr,static_cast<size_t>(fileSize));

}
void HttpResponse::appendResponseBody(Buffer& outBuffer,std::string path){
    size_t fileSize = (size_t)fileIsOK(path_);
    if(fileSize == -1){
        doErrorResponse(outBuffer,"");
        return;
    }
    int fileFd = open(path.data(),O_RDONLY,0);
    void *mmapAddr = mmap(NULL,fileSize,PROT_READ,MAP_PRIVATE,fileFd,0);
    close(fileFd);
    if( mmapAddr == (void*)-1){     //static_cast<void*>(-1) 无法转换
        munmap(mmapAddr,static_cast<size_t>(fileSize));
        outBuffer.retrieveAll();
        statusCode_ = 404;
        doErrorResponse(outBuffer,"Not Found");
        return;
    }
    outBuffer.append(static_cast<char*>(mmapAddr),static_cast<size_t>(fileSize));
    munmap(mmapAddr,static_cast<size_t>(fileSize));
}

std::string HttpResponse::nowTime(){    //struct tm处理时间太难受了，put_time超好用
    /*HTTP-date    = rfc1123-date | rfc850-date | asctime-date
    rfc1123-date = wkday "," SP date1 SP time SP "GMT"
    rfc850-date  = weekday "," SP date2 SP time SP "GMT"
    */
    std::string stringTime;
    time_t t_time;
    time(&t_time);
    std::stringstream sstr;
    sstr << std::put_time(std::localtime(&t_time), "%a, %d %b %Y %X GMT");
    std::getline(sstr,stringTime);
    return stringTime;
}
void HttpResponse::doErrorResponse(Buffer& output,const std::string& message) {     //有时间改成返回404文件

    std::string body;
    auto itr = status2Message.find(statusCode_);
    if(itr == status2Message.end()) {
        itr = status2Message.find(500); //找不到对应状态码，返回500
    }

    body += "<html><title>Server Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    body += std::to_string(statusCode_) + " : " + itr -> second + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em> web server</em></body></html>";
    
    // 响应行
    output.append("HTTP/1.1 " + std::to_string(statusCode_) + " " + itr -> second + "\r\n");
    // 报文头
    output.append("Server:TinyHttpServer\r\n");
    output.append("Content-type:text/html\r\n");
    output.append("Connection:close\r\n");
    output.append("Content-length:" + std::to_string(body.size()) + "\r\n\r\n");
    // 报文体
    output.append(body);   
};

Buffer HttpResponse::makeResponse(){
    Buffer output;
    off_t fileSize = fileIsOK(path_);
    if(fileSize == -1){
        auto it = status2Message.find(statusCode_);
        doErrorResponse(output,it->second);
        return output;
    }

    doStaticResponse(output,fileSize);
    return output;

};

void HttpResponse::doStaticResponse(Buffer& buffer, off_t fileSize){
    appendResponseLine(buffer);
    appendResponseHeader(buffer,fileSize);
    if(!(statusCode_ == 304 || requestMethod_ == "HEAD"))
        appendResponseBody(buffer,fileSize);
};