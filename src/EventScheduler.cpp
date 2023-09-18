#include "EventScheduler.h"

static int createEventFd()
{
    int evtFd = ::eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
    if(evtFd < 0)
    {
        return -1;
    }
    return evtFd;
}

EventScheduler* EventScheduler::createNew(PollerType type)
{
    if(type != POLLER_EPOLL && type != POLLER_POLL && type != POLLER_SELECT)
        return NULL;

    int evtFd = createEventFd();
    if(evtFd < 0)
        return NULL;

    return new EventScheduler(type, evtFd);
}

EventScheduler::EventScheduler(PollerType type, int fd)
    : mIsQuit(false), mWakeupFd(fd)
{
    switch (type)
    {
    case POLLER_SELECT:
        mPoller = SelectPoller::createNew();
        break;
    case POLLER_POLL:
        mPoller = PollPoller::createNew();
    case POLLER_EPOLL:
        mPoller = EpollPoller::createNew();
    default:
        _exit(-1);
        break;
    }

    mTimerManager = TimerManager::createNew(mPoller);
    mWakeIOEvent = IOEvent::createNew(mWakeupFd, this);
    mWakeIOEvent->setReadCallback(handleReadCallback);
    mWakeIOEvent->enableRead();
    mPoller->addIOEvent(mWakeIOEvent);

    mMutex = Mutex::createNew();
}

EventScheduler::~EventScheduler()
{
    mPoller->removeIOEvent(mWakeIOEvent);
    ::close(mWakeupFd);
    delete mWakeIOEvent;
    delete mTimerManager;
    delete mPoller;
}

bool EventScheduler::addTriggerEvent(TriggerEvent* event)
{
    mTriggerEvents.push_back(event);
    return true;
}

Timer::TimerId EventScheduler::addTimerEventRunAfter(TimerEvent* event, Timer::TimeInterval delay)
{
    Timer::Timestamp when = Timer::getCurTime();
    when += delay;

    return mTimerManager->addTimer(event, when, 0);
}

Timer::TimerId EventScheduler::addTimerEventRunAt(TimerEvent* event, Timer::Timestamp when)
{
    return mTimerManager->addTimer(event, when, 0);
}

Timer::TimerId EventScheduler::addTimerEventRunEvery(TimerEvent* event, Timer::TimeInterval interval)
{
    Timer::Timestamp when = Timer::getCurTime();
    when += interval;
    return mTimerManager->addTimer(event, when, interval);
}

bool EventScheduler::removeTimerEvent(Timer::TimerId timerId)
{
    return mTimerManager->removeTimer(timerId);
}

bool EventScheduler::addIOEvent(IOEvent* event)
{
    return mPoller->addIOEvent(event);
}

bool EventScheduler::updateIOEvent(IOEvent* event)
{
    return mPoller->updateIOEvent(event);
}

bool EventScheduler::removeIOEvent(IOEvent* event)
{
    return mPoller->removeIOEvent(event);
}

void EventScheduler::loop()
{
    while(!mIsQuit)
    {
        this->handleTriggerEvents();
        mPoller->handleEvent();
        this->handleOtherEvent();
    }
}

void EventScheduler::wakeup()
{
    uint64_t one = 1;
    int ret;
    ret = ::write(mWakeupFd, &one, sizeof(one));
}

void EventScheduler::handleTriggerEvents()
{
    if(!mTriggerEvents.empty())
    {
        for(auto iter = mTriggerEvents.begin(); iter != mTriggerEvents.end(); ++iter)
            (*iter)->handleEvent();
    }
    mTriggerEvents.clear();
}

void EventScheduler::handleReadCallback(void *arg)
{
    if(arg)
    {
        EventScheduler* scheduler = (EventScheduler*)arg;
        scheduler->handleRead();
    }
}

void EventScheduler::handleRead()
{
    uint64_t one;
    while(::read(mWakeupFd, &one, sizeof(one)) > 0);
}


void EventScheduler::runInLocalThread(Callback callback, void *arg)
{
    MutexLockGuard mutexLockGuard(mMutex);
    mCallbackQueue.emplace(callback, arg);
}

void EventScheduler::handleOtherEvent()
{
    MutexLockGuard mutexLockGuard(mMutex);
    while(!mCallbackQueue.empty())
    {
        auto event = mCallbackQueue.front();
        event.first(event.second);
        mCallbackQueue.pop();
    }
}