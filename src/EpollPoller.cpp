#include "EpollPoller.h"

EpollPoller* EpollPoller::createNew()
{
    return new EpollPoller();
}

EpollPoller::EpollPoller()
    : mEpfd(-1), mEpollEvents(EVENT_INITIAL_SIZE)
{
    mEpfd = epoll_create1(EPOLL_CLOEXEC);
}

EpollPoller::~EpollPoller()
{
    if(mEpfd != -1)
    {
        close(mEpfd);
        mEpfd = -1;
    }
}

bool EpollPoller::addIOEvent(IOEvent* event)
{
    updateIOEvent(event);
}

bool EpollPoller::updateIOEvent(IOEvent* event)
{
    struct epoll_event epEvent;
    int fd = event->getFd();

    memset(&epEvent, 0, sizeof(epEvent));
    epEvent.data.fd = fd;
    if(event->isReadHandling())
        epEvent.events |= EPOLLIN;
    if(event->isWriteHandling())
        epEvent.events |= EPOLLOUT;
    if(event->isErrorHandling())
        epEvent.events |= EPOLLERR;

    if(mEventMap.find(fd) != mEventMap.end())
    {
        if(epoll_ctl(mEpfd, EPOLL_CTL_MOD, fd, &epEvent) == -1)
            return false;
    }
    else
    {
        if(epoll_ctl(mEpfd, EPOLL_CTL_ADD, fd, &epEvent) == -1)
            return false;
        mEventMap.emplace(fd, event);
        if(mEventMap.size() >= mEpollEvents.size())
            mEpollEvents.resize(mEpollEvents.size() * 2);
    }

    return true;
}

bool EpollPoller::removeIOEvent(IOEvent* event)
{
    int fd = event->getFd();
    if(mEventMap.find(fd) == mEventMap.end())
    {
        // 本身不存在
        return false;
    }
    if(epoll_ctl(mEpfd, EPOLL_CTL_DEL, fd, NULL) == -1)
        return false;
    mEventMap.erase(fd);
    return true;
}

void EpollPoller::handleEvent()
{
    int nums = epoll_wait(mEpfd, &*mEpollEvents.begin(), mEpollEvents.size(), 1000);
    if(nums < 0)
    {
        return;
    }
    // epoll_event 转为IOEvent
    int revent, event, fd;
    for(int i = 0; i < nums; ++i)
    {
        revent = 0;
        fd = mEpollEvents[i].data.fd;
        event = mEpollEvents[i].events;
        if(event & EPOLLIN || event & EPOLLPRI || event & EPOLLRDHUP)
            revent |= IOEvent::EVENT_READ;
        if(event & EPOLLOUT)
            revent |= IOEvent::EVENT_WRITE;
        if(event & EPOLLERR)
            revent |= IOEvent::EVENT_ERROR;
    
        auto iter = mEventMap.find(fd);
        if(mEventMap.end() == iter)
            return ;
        iter->second->setREvent(event);
        mEvents.emplace_back(iter->second);
    }

    // 处理IOEvent
    for(auto& it : mEvents)
        it->handleEvent();

    mEvents.clear();
}