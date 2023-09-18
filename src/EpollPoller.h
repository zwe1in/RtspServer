#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H
#include "Poller.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <vector>
#include <string.h>

class EpollPoller : public Poller{
public:
    static EpollPoller* createNew();

    EpollPoller();
    virtual ~EpollPoller();
    
    virtual bool addIOEvent(IOEvent* event);
    virtual bool updateIOEvent(IOEvent* event);
    virtual bool removeIOEvent(IOEvent* event);
    virtual void handleEvent();

private:
    int mEpfd;
    typedef std::vector<struct epoll_event> EPollEventList;
    EPollEventList mEpollEvents;    // 用来接收epoll_wait返回事件epoll_event的数组
    std::vector<IOEvent*> mEvents;  // 用来处理IOEvent的列表

    static const int EVENT_INITIAL_SIZE = 16;
};
#endif