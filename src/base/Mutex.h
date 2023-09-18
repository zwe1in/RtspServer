#ifndef MUTEX_H
#define MUTEX_H
#include <pthread.h>

class Mutex{
public:
    static Mutex* createNew();
    Mutex();
    ~Mutex();

    void lock();
    void unlock();

    pthread_mutex_t* get() { return &mMutex; }
private:
    pthread_mutex_t mMutex;
};

class MutexLockGuard{
public:
    MutexLockGuard(Mutex* mutex);
    ~MutexLockGuard();

private:
    Mutex* mMutex;
};
#endif