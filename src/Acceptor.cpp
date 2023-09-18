#include "Acceptor.h"

Acceptor* Acceptor::createNew(UsageEnvironment* env, const Ipv4Address& addr)
{
    return new Acceptor(env, addr);
}

Acceptor::Acceptor(UsageEnvironment* env, const Ipv4Address& addr)
    : mEnv(env), mAddr(addr), mSocket(sockets::createTpcSocket()), mNewConnectionCallback(NULL)
{
    mSocket.setReuseAddr(1);
    mSocket.bind(mAddr);
    mAcceptIOEvent = IOEvent::createNew(mSocket.getFd(), this);
    mAcceptIOEvent->setReadCallback(readCallback);
    mAcceptIOEvent->enableRead();
}

Acceptor::~Acceptor()
{
    if(mListenning)
        mEnv->scheduler()->removeIOEvent(mAcceptIOEvent);
    delete mAcceptIOEvent;
}

void Acceptor::listen()
{
    mListenning = true;
    mSocket.listen(1024);
    mEnv->scheduler()->addIOEvent(mAcceptIOEvent);
}

void Acceptor::setNewConnectionCallback(NewConnectionCallback cb, void *arg)
{
    mNewConnectionCallback = cb;
    mArg = arg;
}

void Acceptor::readCallback(void *arg)
{
    Acceptor* acceptor = (Acceptor*) arg;
    acceptor->handleRead();
}

void Acceptor::handleRead()
{
    int connFd = mSocket.accept();
    if(mNewConnectionCallback)
        mNewConnectionCallback(mArg, connFd);
}