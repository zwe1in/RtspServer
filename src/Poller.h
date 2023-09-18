#ifndef POLLER_H
#define POLLER_H

#include "Event.h"
#include <map>

class Poller{
public:
    virtual ~Poller() = default;
    
    virtual bool addIOEvent(IOEvent* event) = 0;
    virtual bool updateIOEvent(IOEvent* event) = 0;
    virtual bool removeIOEvent(IOEvent* event) = 0;
    virtual void handleEvent() = 0;
    
protected:
    Poller(){}
protected:
    typedef std::map<int, IOEvent*> IOEventMap;
    IOEventMap mEventMap;
};

#endif