#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include "UsageEnvironment.h"
#include "Event.h"
#include "InetAddress.h"
#include "TcpSocket.h"

class Acceptor{
public:
    typedef void(*NewConnectionCallback)(void* data, int connFd);

    static Acceptor* createNew(UsageEnvironment* env, const Ipv4Address& addr);
    Acceptor(UsageEnvironment* env, const Ipv4Address& addr);
    ~Acceptor();

    bool islistenning() const { return mListenning; }
    void listen();
    void setNewConnectionCallback(NewConnectionCallback cb, void *arg);

private:
    static void readCallback(void*);
    void handleRead();

private:
    UsageEnvironment* mEnv;
    Ipv4Address mAddr;
    IOEvent* mAcceptIOEvent;
    TcpSocket mSocket;
    bool mListenning;
    NewConnectionCallback mNewConnectionCallback;
    void* mArg;
};

#endif