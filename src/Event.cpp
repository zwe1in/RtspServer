#include "Event.h"

TriggerEvent* TriggerEvent::createNew(void *arg)
{
    return new TriggerEvent(arg);
}

TriggerEvent* TriggerEvent::createNew()
{
    return new TriggerEvent(NULL);
}

TriggerEvent::TriggerEvent(void *arg)
    : mArg(arg)
{}

void TriggerEvent::handleEvent()
{
    if(mTriggerCallback)
        mTriggerCallback(mArg);
}

TimerEvent* TimerEvent::createNew(void *arg)
{
    return new TimerEvent(arg);
}

TimerEvent* TimerEvent::createNew()
{
    return new TimerEvent(NULL);
}

TimerEvent::TimerEvent(void *arg)
    : mArg(arg)
{}

void TimerEvent::handleEvent()
{
    if(mTimeoutCallback)
        mTimeoutCallback(mArg);
}

IOEvent* IOEvent::createNew(int fd, void* arg)
{
    if(fd < 0)
    {
        std::cout << "Invalid Fd." << std::endl;
        return nullptr;
    }
    return new IOEvent(fd, arg);
}

IOEvent* IOEvent::createNew(int fd)
{
    if(fd < 0)
    {
        std::cout << "Invalid Fd." << std::endl;
        return nullptr;
    }
    return new IOEvent(fd, nullptr);
}

IOEvent::IOEvent(int fd, void* arg)
    : mFd(fd), mArg(arg), mEvent(EVENT_NONE), mREvent(EVENT_NONE), 
    mReadCallback(nullptr), mWriteCallback(nullptr), mErrorCallback(nullptr)
{}

void IOEvent::handleEvent()
{
    if(mReadCallback && (mREvent & EVENT_READ))
        mReadCallback(mArg);
    if(mWriteCallback && (mREvent & EVENT_WRITE))
        mWriteCallback(mArg);
    if(mErrorCallback && (mREvent & EVENT_ERROR))
        mErrorCallback(mArg);
}