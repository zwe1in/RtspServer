#include "TcpServer.h"

TcpServer::TcpServer(UsageEnvironment* env, const Ipv4Address& addr)
    : mEnv(env), mAddr(addr)
{
    mAcceptor = Acceptor::createNew(env, addr);
    mAcceptor->setNewConnectionCallback(newConnectionCallback, this);
}

TcpServer::~TcpServer()
{
    delete mAcceptor;
}

void TcpServer::start()
{
    mAcceptor->listen();
}

void TcpServer::newConnectionCallback(void *arg, int connFd)
{
    TcpServer* tcpServer = (TcpServer*)arg;
    tcpServer->handleNewConnection(connFd);
}