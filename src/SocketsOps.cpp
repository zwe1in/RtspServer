#include <unistd.h>
#include <sys/types.h>          
#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <string.h>

#include "SocketsOps.h"

int sockets::createTpcSocket()
{
    return ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
}

int sockets::createUdpSocket()
{
    return ::socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
}

bool sockets::bind(int sockFd, std::string ip, uint16_t port)
{
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if(::bind(sockFd, (struct sockaddr*)&addr, sizeof(addr)) < 0)
    {
        // 报错
        return false;
    }

    return true;
}

bool sockets::listen(int sockFd, int backlog)
{
    if(::listen(sockFd, backlog) < 0)
        return false;
    return true;
}

int sockets::accept(int sockFd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrLen = sizeof(addr);
    int conFd = ::accept(sockFd, (struct sockaddr*)&addr, &addrLen);
    setNonBlockAndCloseOnExec(conFd);
    ignoreSigPipeOnSocket(conFd);
    return conFd;
}

void sockets::setNonBlock(int sockFd)
{
    fcntl(sockFd, F_SETFL, fcntl(sockFd, F_GETFL, 0) | O_NONBLOCK);
}

void sockets::setBlock(int sockFd, int timeout)
{
    fcntl(sockFd, F_SETFL, fcntl(sockFd, F_GETFL, 0)&(~O_NONBLOCK));

    if(timeout > 0)
    {
        struct timeval tv = { timeout / 1000,  (timeout % 1000) * 1000};
        setsockopt(sockFd, SOL_SOCKET, SO_SNDTIMEO, (char*)&tv, sizeof(tv));
    }
}

void sockets::setReuseAddr(int sockFd, bool on)
{
    int optval = on ? 1 : 0;
    setsockopt(sockFd, SOL_SOCKET, SO_REUSEADDR, (char*)&optval, sizeof(optval));
}

void sockets::setReusePort(int sockFd)
{
    #ifdef SO_REUSEPORT
        int on = 1;
        setsockopt(sockFd, SOL_SOCKET, SO_REUSEPORT, (char*)&on, sizeof(on));
    #endif
}

void sockets::setNonBlockAndCloseOnExec(int sockFd)
{
    int ret = ::fcntl(sockFd, F_SETFL, ::fcntl(sockFd, F_GETFL, 0) | O_NONBLOCK);
    if(ret < 0)
    {}

    ::fcntl(sockFd, F_SETFD, ::fcntl(sockFd, ::fcntl(sockFd, F_GETFD, 0) | FD_CLOEXEC));
    if(ret < 0)
    {}
}

void sockets::ignoreSigPipeOnSocket(int sockFd)
{
    int optval = 1;
    setsockopt(sockFd, SOL_SOCKET, MSG_NOSIGNAL, &optval, sizeof(optval));
}

void sockets::setNoDelay(int sockFd)
{
    #ifdef TCP_NODELAY
        int optval = 1;
        int ret = setsockopt(sockFd, IPPROTO_TCP, TCP_NODELAY, (char*)&optval, sizeof(optval));
        if(ret < 0)
        {}
    #endif
}

void sockets::setKeepAlive(int sockFd)
{
    int optval = 1;
    int ret = setsockopt(sockFd, SOL_SOCKET, SO_KEEPALIVE, (char*)&optval, sizeof(optval));
    if(ret < 0)
    {}
}

void sockets::setNoSigpipe(int sockFd)
{
    #ifdef SO_NOSIGPIPE
        int optval = 1;
        int ret = setsockopt(sockfd, SOL_SOCKET, SO_NOSIGPIPE, (char *)&optval, sizeof(optval));
        if(ret < 0)
        {}
    #endif
}

void sockets::setSendBufSize(int sockFd, int size)
{
    int ret = setsockopt(sockFd, SOL_SOCKET, SO_SNDBUF, (char*)&size, sizeof(size));
    if(ret < 0)
    {}
}

void sockets::setRecvBufSize(int sockFd, int size)
{
    int ret = setsockopt(sockFd, SOL_SOCKET, SO_RCVBUF, (char*)&size, sizeof(size));
    if(ret < 0)
    {}
}

std::string sockets::getPeerIp(int sockFd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrLen = sizeof(struct sockaddr_in);
    if(getpeername(sockFd, (struct sockaddr*)&addr, &addrLen) == 0)
    {
        return inet_ntoa(addr.sin_addr);
    }
    return "0.0.0.0";
}

uint16_t sockets::getPeerPort(int sockFd)
{
    struct sockaddr_in addr = {0};
    socklen_t addrLen = sizeof(struct sockaddr_in);
    if(getpeername(sockFd, (struct sockaddr*)&addr, &addrLen) == 0)
    {
        return ntohs(addr.sin_port);
    }
    return -1;
}

int sockets::getPeerAddr(int sockFd, struct sockaddr_in* addr)
{
    socklen_t addrLen = sizeof(struct sockaddr_in);
    return getpeername(sockFd, (struct sockaddr*)&addr, &addrLen);
}

void sockets::close(int sockFd)
{
    int ret = ::close(sockFd);
    if(ret < 0)
    {}
}

bool sockets::connect(int sockFd, std::string ip, uint16_t port, int timeout)
{
    bool isConnected = true;
    if(timeout > 0)
        sockets::setNonBlock(sockFd);
    
    struct sockaddr_in addr = {0};
    socklen_t addrLen = sizeof(addr);
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip.c_str());
    addr.sin_port = htons(port);

    if(::connect(sockFd, (struct sockaddr*)&addr, addrLen) < 0)
    {
        if(timeout > 0)
        {
            isConnected = false;
            fd_set fdWrite;
            FD_ZERO(&fdWrite);
            FD_SET(sockFd, &fdWrite);
            struct timeval tv = { timeout / 1000, timeout % 1000 * 1000 };
			select(sockFd + 1, NULL, &fdWrite, NULL, &tv);
            if(FD_ISSET(sockFd, &fdWrite))
                isConnected = true;
            sockets::setBlock(sockFd, 0);
        }
        else
            isConnected = false;
    }
    return isConnected;
}

std::string sockets::getLocalIp()
{
    int sockFd = -1;
    char buf[512] = { 0 };
    struct ifconf ifconf;
    struct ifreq *ifreq;
    sockFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockFd < 0)
    {
        close(sockFd);
        return "0.0.0.0";
    }

    ifconf.ifc_len = 512;
    ifconf.ifc_buf = buf;
    if(ioctl(sockFd, SIOCGIFCONF, &ifconf) < 0)
    {
        close(sockFd);
        return "0.0.0.0";
    }
    close(sockFd);

    ifreq = (struct ifreq*)ifconf.ifc_buf;
    for (int i = (ifconf.ifc_len / sizeof(struct ifreq)); i>0; i--)
    {
        if (ifreq->ifr_flags == AF_INET)
        {
            if (strcmp(ifreq->ifr_name, "lo") != 0)
            {
                return inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr);
            }
            ifreq++;
        }
    }
    return "0.0.0.0";
}

int sockets::readv(int sockFd, const struct iovec* iov, int iovcnt)
{
    return ::readv(sockFd, iov, iovcnt);
}

int sockets::write(int sockFd, const void* buf, int size)
{
    return ::write(sockFd, buf, size);
}

int sockets::sendto(int sockFd, const void* buf, int len, const struct sockaddr* destAddr)
{
    socklen_t addrLen = sizeof(struct sockaddr);
    return ::sendto(sockFd, buf, len, 0, destAddr, addrLen);
}