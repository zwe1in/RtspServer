#ifndef SEM_H
#define SEM_H
#include <semaphore.h>

class Sem{
public:
    static Sem* createNew(int val);

    Sem(int val);
    ~Sem();

    void post();
    void wait();

private:
    sem_t mSem;
};

#endif