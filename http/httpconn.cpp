#include "httpconn.h"

std::atomic<int> HttpConn::userCount;
const char* HttpConn::srcDir=nullptr;

HttpConn::HttpConn()
{
    fd=-1;
    addr={0};
    isClose=true;
    Log::getInstance()->init();
}

HttpConn::~HttpConn()
{
    closeConn();
}

void HttpConn::closeConn()
{
    response.unmapFile();
    if(isClose==false)
    {
        isClose=true;
        userCount--;
        close(fd);
        LOG_INFO("Client (%s:%d) quit, userCount:%d",getIP(),getPort(),static_cast<int>(userCount));
    }
}

void HttpConn::init(int connfd,const sockaddr_in& sockAddr)
{
    assert(connfd>0);
    userCount++;
    fd=connfd;
    addr=sockAddr;
    readBuff.retrieveAll();
    writeBuff.retrieveAll();
    isClose=false;
    LOG_INFO("Client (%s:%d) in, userCount:%d",getIP(),getPort(),static_cast<int>(userCount));
}

const sockaddr_in& HttpConn::getSockAddr() const
{
    return addr;
}
int HttpConn::getFd() const
{
    return fd;
}
int HttpConn::getPort() const
{
    return addr.sin_port;
}
const char* HttpConn::getIP() const
{
    return inet_ntoa(addr.sin_addr);
}
bool HttpConn::isKeepAlive() const
{
    return request.isKeepAlive();
}
bool HttpConn::isWriteOver() const
{
    return vec[0].iov_len+vec[1].iov_len==0;
}

ssize_t HttpConn::read(int* errno_)
{
    ssize_t len=-1;
    while(true)
    {
        len=readBuff.readFd(fd,errno_);
        if(len<=0)
            break;
    }
    return len;
}

ssize_t HttpConn::write(int* errno_)
{
    ssize_t len=-1;
    while (true)
    {
        len=writev(fd,vec,iovCnt);
        if(len<0)
        {
            *errno_=errno;
            break;
        }
        if(vec[0].iov_len+vec[1].iov_len==0) 
        { 
            break; 
        }
        if(static_cast<size_t>(len)>vec[0].iov_len)
        {
            vec[1].iov_base=(uint8_t*)vec[1].iov_base+(len-vec[0].iov_len);
            vec[1].iov_len-=(len-vec[0].iov_len);
            if(vec[0].iov_len)
            {
                writeBuff.retrieveAll();
                vec[0].iov_len=0;
            }
        }
        else
        {
            vec[0].iov_base=(uint8_t*)vec[0].iov_base+len;
            vec[0].iov_len-=len;
            writeBuff.retrieve(len);
        }
    }
    return len;   
}

bool HttpConn::process()
{
    request.init();
    if(readBuff.readableBytes()<=0)
    {
        return false;
    }
    if(request.parse(readBuff))
    {
        LOG_DEBUG("%s", request.getPath().c_str());
        response.init(srcDir,request.getPath(),request.isKeepAlive(),200);
    }
    else
    {
        response.init(srcDir,request.getPath(),false,400);
    }
    response.makeResponse(writeBuff);
    //响应头
    vec[0].iov_base=const_cast<char*>(writeBuff.peek());
    vec[0].iov_len=writeBuff.readableBytes();
    iovCnt=1;
    
    //响应体
    if(response.getFileLen()>0&&response.getfile())
    {
        vec[1].iov_base=response.getfile();
        vec[1].iov_len=response.getFileLen();
        iovCnt=2;
    }
    return true;
}