#ifndef SELECTPOLLER
#define SELECTPOLLER
#include "Poller.h"
#include <sys/select.h>
#include <sys/types.h>
#include <vector>

class SelectPoller : public Poller{
public:
    static SelectPoller* createNew();

    SelectPoller();
    virtual ~SelectPoller();

    virtual bool addIOEvent(IOEvent* event);
    virtual bool updateIOEvent(IOEvent* event);
    virtual bool removeIOEvent(IOEvent* event);
    virtual void handleEvent();

private:
    fd_set mReadSet;
    fd_set mWriteSet;
    fd_set mErrorSet;
    int mMaxSockNum;
    std::vector<IOEvent*> mEvents;
};

#endif