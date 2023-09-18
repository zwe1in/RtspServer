#ifndef TCPSERVER_H
#define TCPSERVER_H
#include "Acceptor.h"
#include "UsageEnvironment.h"
#include "InetAddress.h"
#include "TcpConnection.h"


class TcpServer{
public:
    virtual ~TcpServer();
    void start();
protected:
    TcpServer(UsageEnvironment* env, const Ipv4Address& addr);
    virtual void handleNewConnection(int connFd) = 0;

private:
    static void newConnectionCallback(void *arg, int connFd);
protected:
    UsageEnvironment* mEnv;
    Acceptor* mAcceptor;
    Ipv4Address mAddr;
};

#endif