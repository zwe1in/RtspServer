#include "PollPoller.h"

PollPoller* PollPoller::createNew()
{
    return new PollPoller();
}

PollPoller::PollPoller()
{}

PollPoller::~PollPoller()
{}

bool PollPoller::addIOEvent(IOEvent* event)
{
    updateIOEvent(event);
}

bool PollPoller::updateIOEvent(IOEvent* event)
{
    int fd = event->getFd();
    if(fd < 0)
    {
        return false;
    }

    if(mEventMap.find(fd) != mEventMap.end())
    {
        PollFdMap::iterator it = mPollFdMap.find(fd);
        if(it == mPollFdMap.end())
        {
            return false;
        }

        int index = it->second;
        struct pollfd& pfd = mPollFdList[index];
        pfd.events = 0;
        pfd.revents = 0;

        if(event->isReadHandling())
            pfd.events |= POLLIN;
        if(event->isWriteHandling())
            pfd.events |= POLLOUT;
        if(event->isErrorHandling())
            pfd.events |= POLLERR;
    }
    else
    {
        struct pollfd pfd;
        pfd.fd = fd;
        pfd.events = 0;
        pfd.revents = 0;

        if(event->isReadHandling())
            pfd.events |= POLLIN;
        if(event->isWriteHandling())
            pfd.events |= POLLOUT;
        if(event->isErrorHandling())
            pfd.events |= POLLERR;

        mPollFdList.push_back(pfd);
        mEventMap.emplace(fd, event);
        mPollFdMap.emplace(fd, mPollFdList.size() - 1);
    }
    return true;
}

bool PollPoller::removeIOEvent(IOEvent* event)
{
    int fd = event->getFd();

    if(mEventMap.find(fd) == mEventMap.end())
    {
        return false;
    }

    PollFdMap::iterator it = mPollFdMap.find(fd);
    if(it == mPollFdMap.end())
    {
        return false;
    }
    int index = it->second;
    /* 确保删除的元素是位于末尾 */
    if(index != mPollFdList.size() - 1)
    {
        std::iter_swap(mPollFdList.begin() + index, mPollFdList.end() - 1);
        int tmpFd = mPollFdList[index].fd;
        mPollFdMap.find(tmpFd)->second = index;
    }

    mPollFdList.pop_back();
    mPollFdMap.erase(fd);
    mEventMap.erase(fd);

    return true;
}

void PollPoller::handleEvent()
{
    if(mPollFdList.empty())
        return ;

    int num = poll(&*mPollFdList.begin(), mPollFdList.size(), 1000);
    if(num < 0)
    {
        return ;
    }

    int cbEvent = 0, revents = 0;
    for(auto it = mPollFdList.begin(); it != mPollFdList.end() && num > 0; ++it)
    {
        cbEvent = it->revents;
        if(cbEvent > 0)
        {
            revents = 0;
            auto iter = mEventMap.find(it->fd);
            if(iter == mEventMap.end())
                return ;
            if(cbEvent & POLLIN || cbEvent & POLLHUP || cbEvent & POLLPRI)
                revents |= IOEvent::EVENT_READ;
            if(cbEvent & POLLOUT)
                revents |= IOEvent::EVENT_WRITE;
            if(cbEvent & POLLERR)
                revents |= IOEvent::EVENT_ERROR;
            
            iter->second->setREvent(revents);
            mEvents.push_back(iter->second);
            --num;
        }
    }

    for(auto it = mEvents.begin(); it != mEvents.end(); ++it)
        (*it)->handleEvent();
    mEvents.clear();
}