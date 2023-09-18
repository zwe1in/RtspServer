#ifndef USAGEENVIRONMENT_H
#define USAGEENVIRONMENT_H
#include "EventScheduler.h"
#include "base/ThreadPool.h"

class UsageEnvironment{
public:
    static UsageEnvironment* createNew(EventScheduler* scheduler, ThreadPool* threadPool);

    UsageEnvironment(EventScheduler* scheduler, ThreadPool* threadPool);
    ~UsageEnvironment();

    EventScheduler* scheduler() const { return mScheduler; }
    ThreadPool* threadPool() const { return mThreadPool; }

private:
    EventScheduler* mScheduler;
    ThreadPool* mThreadPool;
};

#endif