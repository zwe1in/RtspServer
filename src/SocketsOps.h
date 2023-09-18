#ifndef SOCKETSOPS_H
#define SOCKETSOPS_H
#include <string>
#include <arpa/inet.h>
#include <sys/uio.h>

namespace sockets{

int createTpcSocket();
int createUdpSocket();
bool bind(int sockFd, std::string ip, uint16_t port);
bool listen(int sockFd, int backlog);
int accept(int sockFd);

void setNonBlock(int sockFd);
void setBlock(int sockFd, int timeout);
void setReuseAddr(int sockFd, bool on);
void setReusePort(int sockFd);
void setNoDelay(int sockFd);
void setKeepAlive(int sockFd);
void setNoSigpipe(int sockFd);
void setSendBufSize(int sockFd, int size);
void setRecvBufSize(int sockFd, int size);
void setNonBlockAndCloseOnExec(int sockFd);
void ignoreSigPipeOnSocket(int sockFd);
std::string getPeerIp(int sockFd);
uint16_t getPeerPort(int sockFd);
int getPeerAddr(int sockFd, struct sockaddr_in* addr);
void close(int sockFd);
bool connect(int sockFd, std::string ip, uint16_t port, int timeout);
std::string getLocalIp();
int readv(int sockFd, const struct iovec* iov, int iovcnt);
int write(int sockFd, const void* buf, int size);
int sendto(int sockFd, const void* buf, int len, const struct sockaddr* destAddr);

};

#endif