#include "TcpSocket.h"

TcpSocket::~TcpSocket()
{
    sockets::close(mSockFd);
}

bool TcpSocket::bind(Ipv4Address& addr)
{
    return sockets::bind(mSockFd, addr.getIp(), addr.getPort());
}

bool TcpSocket::listen(int backlog)
{
    return sockets::listen(mSockFd, backlog);
}

int TcpSocket::accept()
{
    return sockets::accept(mSockFd);
}

void TcpSocket::setReuseAddr(int on)
{
    sockets::setReuseAddr(mSockFd, on);
}

void TcpSocket::setNonBlock()
{
    sockets::setNonBlock(mSockFd);
}