#ifndef EVENT_H
#define EVENT_H
#include <iostream>

typedef void (*EventCallback)(void*);

class TriggerEvent{
public:
    static TriggerEvent* createNew(void* arg);
    static TriggerEvent* createNew();

    TriggerEvent(void *arg);
    ~TriggerEvent(){ }

    void setArg(void *arg){ mArg = arg; }
    void setTriggerCallback(EventCallback cb) { mTriggerCallback = cb; }
    void handleEvent();

private:
    void *mArg;
    EventCallback mTriggerCallback;
};

class TimerEvent{
public:
    static TimerEvent* createNew(void *arg);
    static TimerEvent* createNew();

    TimerEvent(void *arg);
    ~TimerEvent() = default;

    void setArg(void *arg){ mArg = arg; }
    void setTimeoutCallback(EventCallback cb) { mTimeoutCallback = cb; }
    void handleEvent();

private:
    void *mArg;
    EventCallback mTimeoutCallback;
};


class IOEvent{
public:
    enum IOEventType
    {
        EVENT_NONE = 0,
        EVENT_READ = 1,
        EVENT_WRITE = 2,
        EVENT_ERROR = 4
    };

    static IOEvent* createNew(int fd, void* arg);
    static IOEvent* createNew(int fd);

    IOEvent(int fd, void* arg);
    ~IOEvent() = default;

    int getFd() const { return mFd; }
    int getmEvent() const { return mEvent; }
    int getmREvent() const { return mREvent; }
    
    void setREvent(int event) { mREvent = event; }
    void setArg(void* arg) { mArg = arg; }
    void setReadCallback(EventCallback cb) { mReadCallback = cb; }
    void setWriteCallback(EventCallback cb) { mWriteCallback = cb; }
    void setErrorCallback(EventCallback cb) { mErrorCallback = cb; }

    void enableRead() { mEvent |= EVENT_READ; }
    void enableWrite() { mEvent |= EVENT_WRITE; }
    void enableError() { mEvent |= EVENT_ERROR; }
    void disableRead() { mEvent &= ~EVENT_READ; }
    void disableWrite() { mEvent &= ~EVENT_WRITE; }
    void disableError() { mEvent &= ~EVENT_ERROR; }

    bool isNoneHandling() const { return mEvent == EVENT_NONE; }
    bool isReadHandling() const { return mEvent & EVENT_READ; }
    bool isWriteHandling() const { return mEvent & EVENT_WRITE; }
    bool isErrorHandling() const { return mEvent & EVENT_ERROR; }

    void handleEvent();

private:
    int mFd;
    void* mArg;
    int mEvent;
    int mREvent;
    EventCallback mReadCallback;
    EventCallback mWriteCallback;
    EventCallback mErrorCallback;

};

#endif