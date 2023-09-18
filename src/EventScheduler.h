#ifndef EVENT_SCHEDULER_H
#define EVENT_SCHEDULER_H
#include <vector>
#include <queue>
#include <sys/eventfd.h>

#include "Poller.h"
#include "PollPoller.h"
#include "EpollPoller.h"
#include "SelectPoller.h"
#include "base/Mutex.h"
#include "Timer.h"

class EventScheduler{
public:
    typedef void (*Callback)(void*);

    enum PollerType{
        POLLER_SELECT,
        POLLER_POLL,
        POLLER_EPOLL
    };

    static EventScheduler* createNew(PollerType type);

    EventScheduler(PollerType type, int fd);
    virtual ~EventScheduler();

    bool addTriggerEvent(TriggerEvent* event);
    Timer::TimerId addTimerEventRunAfter(TimerEvent* event, Timer::TimeInterval delay);
    Timer::TimerId addTimerEventRunAt(TimerEvent* event, Timer::Timestamp when);
    Timer::TimerId addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval);
    bool removeTimerEvent(Timer::TimerId timerId);

    bool addIOEvent(IOEvent* event);
    bool updateIOEvent(IOEvent* event);
    bool removeIOEvent(IOEvent* event);

    void loop();
    void wakeup();
    void runInLocalThread(Callback callback, void *arg);
    void handleOtherEvent();

private:
    void handleTriggerEvents();
    static void handleReadCallback(void*);
    void handleRead();

private:
    bool mIsQuit;
    Poller* mPoller;
    TimerManager* mTimerManager;
    std::vector<TriggerEvent*> mTriggerEvents;
    int mWakeupFd;
    IOEvent* mWakeIOEvent;

    std::queue<std::pair<Callback, void*>> mCallbackQueue;
    Mutex* mMutex;
};


#endif