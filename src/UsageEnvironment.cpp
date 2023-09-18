#include "UsageEnvironment.h"

UsageEnvironment* UsageEnvironment::createNew(EventScheduler* scheduler, ThreadPool* threadPool)
{
    if(!scheduler)
        return NULL;

    return new UsageEnvironment(scheduler, threadPool);
}

UsageEnvironment::UsageEnvironment(EventScheduler* scheduler, ThreadPool* threadPool)
    : mScheduler(scheduler), mThreadPool(threadPool)
{}

UsageEnvironment::~UsageEnvironment() 
{
    delete mScheduler;
    delete mThreadPool;
}