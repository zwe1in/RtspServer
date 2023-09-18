#include "SelectPoller.h"
#include "Logging.h"

static const int selectTimeout = 10000;

SelectPoller* SelectPoller::createNew()
{
    return new SelectPoller();
}

SelectPoller::SelectPoller()
{
    FD_ZERO(&mReadSet);
    FD_ZERO(&mWriteSet);
    FD_ZERO(&mErrorSet);
}


SelectPoller::~SelectPoller()
{}

bool SelectPoller::addIOEvent(IOEvent* event)
{
    return updateIOEvent(event);
}

bool SelectPoller::updateIOEvent(IOEvent* event)
{
    int fd = event->getFd();
    if(fd < 0)
    {
        LOG_WARNING("Failed to add ioEvent\n");
        return false;
    }

    FD_CLR(fd, &mReadSet);
    FD_CLR(fd, &mWriteSet);
    FD_CLR(fd, &mErrorSet);

    IOEventMap::iterator iter = mEventMap.find(fd);

    if(iter != mEventMap.end())
    {
        if(event->isReadHandling())
            FD_SET(fd, &mReadSet);
        if(event->isWriteHandling())
            FD_SET(fd, &mWriteSet);
        if(event->isErrorHandling())
            FD_SET(fd, &mErrorSet);
    }
    else
    {
        if(event->isReadHandling())
            FD_SET(fd, &mReadSet);
        if(event->isWriteHandling())
            FD_SET(fd, &mWriteSet);
        if(event->isErrorHandling())
            FD_SET(fd, &mErrorSet);

        mEventMap.insert(std::make_pair(fd, event));
    }

    if(mEventMap.empty())
        mMaxSockNum = 0;
    else
        mMaxSockNum = mEventMap.rbegin()->first + 1;    
    
    return true;
}

bool SelectPoller::removeIOEvent(IOEvent* event)
{
    int fd = event->getFd();
    if(fd < 0)
        return false;
    
    FD_CLR(fd, &mReadSet);
    FD_CLR(fd, &mWriteSet);
    FD_CLR(fd, &mErrorSet);

    IOEventMap::iterator iter = mEventMap.find(fd);
    if(mEventMap.end() != iter)
        mEventMap.erase(iter);

    if(mEventMap.empty())
        mMaxSockNum = 0;
    else
        mMaxSockNum = mEventMap.rbegin()->first + 1;

    return true;
}

void SelectPoller::handleEvent()
{
    fd_set readSet = mReadSet;
    fd_set writeSet = mWriteSet;
    fd_set errorSet = mErrorSet;

    struct timeval timeout;
    timeout.tv_sec = selectTimeout;
    timeout.tv_usec = 0;
    int ret = select(mMaxSockNum, &readSet, &writeSet, &errorSet, &timeout);

    if(ret < 0)
    {
        LOG_ERROR("Select error\n");
        return ;
    }

    int rEvent;
    for(IOEventMap::iterator iter = mEventMap.begin(); iter != mEventMap.end(); ++iter)
    {
        rEvent = 0;
        if(FD_ISSET(iter->first, &readSet))
            rEvent |= IOEvent::EVENT_READ;
        if(FD_ISSET(iter->first, &writeSet))
            rEvent |= IOEvent::EVENT_WRITE;
        if(FD_ISSET(iter->first, &errorSet))
            rEvent |= IOEvent::EVENT_ERROR;
        if(rEvent != 0)
        {
            iter->second->setREvent(rEvent);
            mEvents.push_back(iter->second);
        }
    }

    for(auto iter = mEvents.begin(); iter != mEvents.end(); ++iter)
        (*iter)->handleEvent();
    
    mEvents.clear();
}