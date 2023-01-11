#include "epoller.h"

Epoller::Epoller(int maxEvent):epollFd(epoll_create(512)),events(maxEvent)
{
    assert(epollFd>=0&&events.size()>0);
}

Epoller::~Epoller()
{
    close(epollFd);
}

bool Epoller::addFd(int fd,uint32_t events)
{
    if(fd<0)
        return false;
    epoll_event event= {0};
    event.data.fd=fd;
    event.events=events;
    return epoll_ctl(epollFd,EPOLL_CTL_ADD,fd,&event)==0;
}

bool Epoller::modFd(int fd,uint32_t events)
{
    if(fd<0)
        return false;
    epoll_event event= {0};
    event.data.fd=fd;
    event.events=events;
    return epoll_ctl(epollFd,EPOLL_CTL_MOD,fd,&event)==0;
}

bool Epoller::delFd(int fd)
{
    if(fd<0)
        return false;
    epoll_event event= {0};
    return epoll_ctl(epollFd,EPOLL_CTL_DEL,fd,&event)==0;
}

int Epoller::wait(int timeoutMs)
{
   return epoll_wait(epollFd,&events[0],static_cast<int>(events.size()),timeoutMs);
}

int Epoller::getEventFd(size_t i) const
{
    assert(i<events.size()&&i>=0);
    return events[i].data.fd;
}

uint32_t Epoller::getEvents(size_t i) const
{
    assert(i<events.size()&&i>=0);
    return events[i].events;   
}