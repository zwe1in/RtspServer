#include "TcpConnection.h"

TcpConnection::TcpConnection(UsageEnvironment* env, int sockFd)
    : mEnv(env), mSocket(sockFd), mDisconnectionCallback(NULL), mArg(NULL)
{
    mTcpConnIOEvent = IOEvent::createNew(sockFd, this);
    mTcpConnIOEvent->setReadCallback(readCallback);
    mTcpConnIOEvent->setWriteCallback(writeCallback);
    mTcpConnIOEvent->setErrorCallback(errorCallback);
    mTcpConnIOEvent->enableRead();
    mEnv->scheduler()->addIOEvent(mTcpConnIOEvent);
}

TcpConnection::~TcpConnection()
{
    mEnv->scheduler()->removeIOEvent(mTcpConnIOEvent);
    delete mTcpConnIOEvent;
}

void TcpConnection::setDisconnectionCallback(DisconnectionCallback cb, void *arg)
{
    mDisconnectionCallback = cb;
    mArg = arg;
}

void TcpConnection::enableReadHandling()
{
    if(mTcpConnIOEvent->isReadHandling())
        return ;

    mTcpConnIOEvent->enableRead();
    mEnv->scheduler()->updateIOEvent(mTcpConnIOEvent);
}   

void TcpConnection::enableWriteHandling()
{
    if(mTcpConnIOEvent->isWriteHandling())
        return ;
    
    mTcpConnIOEvent->enableWrite();
    mEnv->scheduler()->updateIOEvent(mTcpConnIOEvent);
}

void TcpConnection::enableErrorHandling()
{
    if(mTcpConnIOEvent->isErrorHandling())
        return ;

    mTcpConnIOEvent->enableError();
    mEnv->scheduler()->updateIOEvent(mTcpConnIOEvent);
}

void TcpConnection::disableReadHandling()
{
    if(!mTcpConnIOEvent->isReadHandling())
        return ;

    mTcpConnIOEvent->disableRead();
    mEnv->scheduler()->updateIOEvent(mTcpConnIOEvent);
}

void TcpConnection::disableWriteHandling()
{
    if(!mTcpConnIOEvent->isWriteHandling())
        return ;

    mTcpConnIOEvent->disableWrite();
    mEnv->scheduler()->updateIOEvent(mTcpConnIOEvent);
}

void TcpConnection::disableErrorHandling()
{
    if(!mTcpConnIOEvent->isErrorHandling())
        return ;
    
    mTcpConnIOEvent->disableError();
    mEnv->scheduler()->updateIOEvent(mTcpConnIOEvent);
}

void TcpConnection::handleRead()
{
    int ret = mInputBuffer.read(mSocket.getFd());

    if(ret == 0)
    {
        printf("Disconnection.\n");
        handleDisconnection();
        return ;
    }
    else if(ret < 0)
    {
        printf("Read error.\n");
        handleDisconnection();
    }
    handleReadBytes();
}

void TcpConnection::handleReadBytes()
{
    mInputBuffer.retrieveAll();
}

void TcpConnection::handleWrite()
{
    mOutputBuffer.retrieveAll();
}

void TcpConnection::handleError()
{
    printf("Error.\n");
}

void TcpConnection::readCallback(void *arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleRead();
}

void TcpConnection::writeCallback(void *arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleWrite();
}

void TcpConnection::errorCallback(void *arg)
{
    TcpConnection* tcpConnection = (TcpConnection*)arg;
    tcpConnection->handleError();
}

void TcpConnection::handleDisconnection()
{
    if(mDisconnectionCallback)
        mDisconnectionCallback(mArg, mSocket.getFd());
}