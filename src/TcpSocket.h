#ifndef TCPSOCKET_H
#define TCPSOCKET_H
#include "SocketsOps.h"
#include "InetAddress.h"
#include <string>

class TcpSocket{
public:
    explicit TcpSocket(int sockFd) : mSockFd(sockFd) {}
    ~TcpSocket();

    int getFd() const { return mSockFd; }
    bool bind(Ipv4Address& addr);
    bool listen(int backlog);
    int accept();
    void setReuseAddr(int on);
    void setNonBlock();
private:
    int mSockFd;
};

#endif