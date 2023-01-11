#ifndef HTTPCONN_H
#define HTTPCONN_H

#include<arpa/inet.h>
#include<errno.h>
#include <atomic>

#include "../log/log.h"
#include "../pool/sqlconnpool.h"
#include "../buffer/buffer.h"
#include "httprequest.h"
#include "httpresponse.h"

class HttpConn
{
public:
    HttpConn();
    ~HttpConn();
    void init(int connfd,const sockaddr_in& sockAddr);
    ssize_t read(int* errno_);
    ssize_t write(int* errno_);
    void closeConn();

    const sockaddr_in& getSockAddr() const;
    int getFd() const;
    int getPort() const;
    const char* getIP() const;
    bool isKeepAlive() const;
    bool isWriteOver() const;

    bool process();

    static std::atomic<int> userCount;
    static const char* srcDir;
private:
    int fd;
    struct sockaddr_in addr;
    bool isClose;
    int iovCnt;
    struct  iovec vec[2];

    Buffer readBuff;
    Buffer writeBuff;
    HttpRequest request;
    HttpResponse response;
};

#endif //! HTTPCONN__H