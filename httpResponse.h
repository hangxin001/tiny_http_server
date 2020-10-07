#pragma once
#include"buffer.h"
#include<unordered_map>
#include<string>
class HttpResponse{
public:
    static const std::unordered_map<int, std::string> status2Message;
    static const std::unordered_map<std::string, std::string> file2Type;

    static const int CONNECT_TIMEOUT = 500; //MS
    HttpResponse(int statusCode , std::string path,std::string requestMethod ,bool keepAlive);
    HttpResponse(int statusCode , std::string path,std::string requestMethod , 
    std::unordered_map<std::string,std::string> requestHeader, bool keepAlive); //检查requestHeader是否需要回应部分头部
    Buffer makeResponse();
    void doErrorResponse(Buffer& buffer, const std::string& message);
    void doStaticResponse(Buffer& buffer, off_t fileSize);  //stat.st_size类型为off_t
    void doNotModifiedResponse(Buffer& outBuffer);
private:
    std::string getFileType();
    off_t fileIsOK(std::string path); //检查文件是否存在以及有权限,并返回文件长度,错误会改变statusCode_
    void appendResponseLine(Buffer& outBuffer);
    void appendResponseHeader(Buffer& outBuffer, off_t fileSize);
    void appendResponseBody(Buffer& outBuffer, off_t fileSize);  //默认文件为path_
    void appendResponseBody(Buffer& outBuffer,std::string path);
    std::string nowTime();  //返回互联网的标准格式时间格式RFC 1123
private:
    int statusCode_;
    std::string path_;
    bool keepAlive_;
    std::string requestMethod_;     //如果为HEAD则不添加REsponseBody
    std::unordered_map<std::string,std::string> requestHeader_;     // 传指针或引用效率搞，并且不用当心提前释放问题
};