#include "webserver.h"

bool WebServer::loadConfigFile()
{
    FILE* fp = fopen("./webserver.ini", "r");
    if(fp==nullptr)
    {
        return false;
    }    
    while(!feof(fp)) 
    {
    	char line[1024] = {0};
        fgets(line, 1024, fp);
        string str = line;
        int idx = str.find('=', 0);
        if (idx == -1)
            continue;
        int endidx = str.find('\n', idx);
        string key = str.substr(0, idx);
        string value = str.substr(idx+1,endidx-idx-1);
        if (key == "port")    
            port = stoi(value);
        else if (key == "timeOutMs")
            timeOutMs = stoi(value);
        else if (key == "sqlConnMaxNum")
            SqlConnPool::getInstance(stoi(value));
        else if (key == "threadNum") 
            threadPool_p=std::unique_ptr<ThreadPool>(new ThreadPool(stoi(value)));
        else if (key == "logQueSize")
            Log::getInstance()->init(stoi(value));
    }
    fclose(fp);
    return true;
}

WebServer::WebServer():isClose(false),timer_p(new HeapTimer()),epoller_p(new Epoller())
{
    if (!loadConfigFile())//配置失败
    {
        isClose=true;
        LOG_ERROR("%s","Load Config File Fail!");
        return ;
    }
    srcDir=getcwd(nullptr,256);
    strncat(srcDir,"/resources/",15);
    HttpConn::userCount=0;
    HttpConn::srcDir=srcDir;
    listenEvent=EPOLLRDHUP|EPOLLIN|EPOLLET;
    connEvent=EPOLLONESHOT|EPOLLRDHUP|EPOLLET;
    if(!initSocket())
        isClose=true;
    if(isClose)
        LOG_ERROR("%s","========== Server init error!==========");
    else
        LOG_INFO("%s", "========== Server init success!========");
}

WebServer::~WebServer()
{
    close(listenFd);
    isClose=true;
    free(srcDir);
}

void WebServer::start() 
{
    int timeMs=-1;
    while(!isClose)
    {
        if(timeOutMs>0)
        {
            timeMs=timer_p->getNextTick();
        }
        int eventCnt=epoller_p->wait(timeMs);
        for(int i=0;i<eventCnt;i++)
        {
            int fd=epoller_p->getEventFd(i);
            uint32_t events=epoller_p->getEvents(i);
            if(fd==listenFd)
            {
                dealListen();
            }
            else if(events&EPOLLIN)
            {
                dealRead(&users[fd]);
            }
            else if(events&EPOLLOUT)
            {
                dealWrite(&users[fd]);
            }
            else if(events&(EPOLLRDHUP|EPOLLHUP|EPOLLERR))
            {
                closeConn(&users[fd]);
            }
            else
            {
                LOG_ERROR("%s","Unexpected Event Happen!");
            }
        }
    }
    
}

void WebServer::dealListen()
{
    struct sockaddr_in addr;
    socklen_t len=sizeof(addr);
    while(true)
    {
        int connfd=accept(listenFd,(struct sockaddr*)&addr,&len);
        if(HttpConn::userCount>=maxUserNum)
        {
            close(connfd);
            LOG_ERROR("%s","Server Users Full!");
            return ;
        }
        if(connfd<=0)
        {
            return ;
        }
        else
        {
            addClient(connfd,addr);
        }
    }
}

 void WebServer::dealRead(HttpConn* client)
 {
    flushTime(client);
    threadPool_p->addTask(std::bind(&WebServer::readFrom,this,client));
 }

void WebServer::dealWrite(HttpConn* client)
{
    flushTime(client);
    threadPool_p->addTask(std::bind(&WebServer::writeTo,this,client));
}

void WebServer::closeConn(HttpConn* client)
{
    epoller_p->delFd(client->getFd());
    client->closeConn();
}

void WebServer::flushTime(HttpConn* client)
{
    if(timeOutMs>0)
    {
        timer_p->adjust(client->getFd(),timeOutMs);
    }
}

void WebServer::addClient(int connfd,const sockaddr_in& addr)
{
    users[connfd].init(connfd,addr);
    if(timeOutMs>0)
    {
        timer_p->add(connfd,timeOutMs,std::bind(&WebServer::closeConn,this,&users[connfd]));
    }
    epoller_p->addFd(connfd,connEvent|EPOLLIN);
    setFdNonblock(connfd);
}

void WebServer::readFrom(HttpConn* client)
{
    int readErrno=0;
    int ret=client->read(&readErrno);
    if(ret<=0&&readErrno!=EAGAIN)
    {
        closeConn(client);
        return ;
    }
    process(client);
}

void WebServer::writeTo(HttpConn* client)
{
    int writeErrno=0;
    int ret=client->write(&writeErrno);
    if(client->isWriteOver()) 
    {
        if(client->isKeepAlive()) 
        {
            process(client);
            return;
        }
    }
    if(ret<0&&writeErrno==EAGAIN) 
    {
        epoller_p->modFd(client->getFd(), connEvent | EPOLLOUT);
        return ;
    }
    closeConn(client);
}

void WebServer::process(HttpConn* client)
{
    if(client->process()) 
    {
        epoller_p->modFd(client->getFd(), connEvent | EPOLLOUT);
    } 
    else 
    {
        epoller_p->modFd(client->getFd(), connEvent | EPOLLIN);
    }
}

bool WebServer::setFdNonblock(int fd)
{
    int flags;
    if((flags=fcntl(fd,F_GETFL,0))<0)    
        return false;
    flags |= O_NONBLOCK;
    if(fcntl(fd,F_SETFL,flags)<0)
        return false;
    return true;
}

bool WebServer::initSocket()
{
    if(port>65535||port<1024)
    {
        LOG_ERROR("Select Port:%d Error!",port);
        return false;
    }
    listenFd=socket(AF_INET,SOCK_STREAM,0);
    if(listenFd<0)
    {
        LOG_ERROR("%s","Create Socket Error!");
        return false;
    }
    struct linger optLinger;
    optLinger.l_onoff=1;
    optLinger.l_linger=1;
    if(setsockopt(listenFd,SOL_SOCKET,SO_LINGER,&optLinger,sizeof(optLinger))<0)
    {
        LOG_ERROR("%s","Set SO_LINGER Option Error!");
        return false;
    }
    int optReuseaddr=1;
    if(setsockopt(listenFd,SOL_SOCKET,SO_REUSEADDR,&optReuseaddr,sizeof(optReuseaddr))<0)
    {
        LOG_ERROR("%s","Set SO_REUSEADDR Option Error!");
        return false;
    }
    struct sockaddr_in addr;
    addr.sin_family=AF_INET;
    addr.sin_addr.s_addr=htonl(INADDR_ANY);
    addr.sin_port=htons(port);
    if(bind(listenFd,(struct sockaddr*)&addr,sizeof(addr))<0)
    {
        LOG_ERROR("%s","Bind Port:%d Error!",port);
        return false;
    }
    if(listen(listenFd,5)<0)
    {
        LOG_ERROR("%s","Listen Port:%d Error!",port);
        return false;
    }
    if(!epoller_p->addFd(listenFd,listenEvent))
    {
        LOG_ERROR("%s","Add ListenFd:%d in Epoll Error!",listenFd);
        return false;
    }
    if(!setFdNonblock(listenFd))
    {
        LOG_ERROR("%s","Set ListenFd:%d Nonblock Error!",listenFd);
        return false;
    }
    return true;
}