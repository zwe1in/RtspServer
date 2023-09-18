#ifndef CONDITION_H
#define CONDITION_H
#include "Mutex.h"

class Condition{
public:
    static Condition* createNew();

    Condition();
    ~Condition();

    void wait(Mutex* mutex);
    bool waitTimeout(Mutex* mutex, int ms);
    void signal();
    void broadcast();
private:
    pthread_cond_t mCond;
};

#endif