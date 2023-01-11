#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unordered_map>
#include <errno.h>
#include <assert.h>

#include "heaptimer.h"
#include "epoller.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnpool.h"
#include "../http/httpconn.h"
#include "../log/log.h"

class  WebServer
{
public:
    WebServer();
    ~WebServer();
    bool loadConfigFile();
    void start();

private:
    void dealListen();
    void dealRead(HttpConn* client);
    void dealWrite(HttpConn* client);
    void closeConn(HttpConn* client);

    void flushTime(HttpConn* client);
    void addClient(int fd,const sockaddr_in& addr);
    void readFrom(HttpConn* client);
    void writeTo(HttpConn* client);
    void process(HttpConn* client);

    bool setFdNonblock(int fd);
    bool initSocket();
    
    int port;
    int timeOutMs;
    bool isClose;
    int listenFd;
    char* srcDir;

    uint32_t listenEvent;
    uint32_t connEvent;
    std::unique_ptr<HeapTimer> timer_p;
    std::unique_ptr<ThreadPool> threadPool_p;
    std::unique_ptr<Epoller> epoller_p;
    std::unordered_map<int,HttpConn> users;
    const int maxUserNum = 65536;
};

#endif // !WEBSERVER_H