#ifndef TCPCONNECTION_H
#define TCPCONNECTION_H
#include "UsageEnvironment.h"
#include "Event.h"
#include "TcpSocket.h"
#include "Buffer.h"

class TcpConnection{
public:
    typedef void (*DisconnectionCallback)(void*, int);

    TcpConnection(UsageEnvironment* env, int sockFd);
    virtual ~TcpConnection();

    void setDisconnectionCallback(DisconnectionCallback cb, void *arg);

protected:
    void enableReadHandling();
    void enableWriteHandling();
    void enableErrorHandling();
    void disableReadHandling();
    void disableWriteHandling();
    void disableErrorHandling();
    
    void handleRead();
    virtual void handleReadBytes();
    virtual void handleWrite();
    virtual void handleError();

    void handleDisconnection();

private:
    static void readCallback(void *arg);
    static void writeCallback(void *arg);
    static void errorCallback(void *arg);

protected:
    UsageEnvironment* mEnv;
    TcpSocket mSocket;
    IOEvent* mTcpConnIOEvent;
    DisconnectionCallback mDisconnectionCallback;
    void* mArg;
    Buffer mInputBuffer;
    Buffer mOutputBuffer;
    char mBuffer[2048];
};
#endif