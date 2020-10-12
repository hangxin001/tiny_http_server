#include"httpServer.h"
#include<cstring>
int main(int argc, char** argv){
    std::string ip = "0.0.0.0";
    int port = 33333;
    if(argc == 2){
        ip = std::string(*(argv+1),strlen(*(argv+1)));
    }

    HttpServer server(ip,port,6);
    server.run();
}