#include <algorithm>
#include <assert.h>
#include <stdio.h>

#include "Logging.h"
#include "RtspServer.h"


RtspServer* RtspServer::createNew(UsageEnvironment* env, Ipv4Address addr)
{
    return new RtspServer(env, addr);
}

RtspServer::RtspServer(UsageEnvironment* env, const Ipv4Address& addr)
    : TcpServer(env, addr)
{
    mTriggerEvent = TriggerEvent::createNew(this);
    mTriggerEvent->setTriggerCallback(triggerCallback);

    mMutex = Mutex::createNew();
}

RtspServer::~RtspServer()
{
    delete mTriggerEvent;
    delete mMutex;
}

void RtspServer::handleNewConnection(int connFd)
{
    RtspConnection* conn = RtspConnection::createNew(this,connFd);
    conn->setDisconnectionCallback(disconnectionCallback, this);
    mConnections.insert(std::make_pair(connFd, conn));
}

void RtspServer::disconnectionCallback(void *arg, int sockFd)
{
    RtspServer* rtspServer = (RtspServer*)arg;
    rtspServer->handleDisconnection(sockFd);
}

void RtspServer::handleDisconnection(int sockFd)
{
    MutexLockGuard mutexLockGuard(mMutex);
    mDisconnectionList.push_back(sockFd);
    mEnv->scheduler()->addTriggerEvent(mTriggerEvent);
}

void RtspServer::triggerCallback(void* arg)
{
    RtspServer* rtspServer = (RtspServer*)arg;
    rtspServer->handleDisconnectionList();
}

void RtspServer::handleDisconnectionList()
{
    MutexLockGuard mutexLockGuard(mMutex);
    for(auto iter = mDisconnectionList.begin(); iter != mDisconnectionList.end(); ++iter)
    {
        int sockFd = *iter;
        std::map<int, RtspConnection*>::iterator conn_it = mConnections.find(sockFd);
        assert(conn_it != mConnections.end());

        delete conn_it->second;
        mConnections.erase(sockFd);
    }
    mDisconnectionList.clear();
}

bool RtspServer::addMediaSession(MediaSession* mediaSession)
{
    if(mMediaSessions.find(mediaSession->getSessionName()) != mMediaSessions.end())
        return false;

    mMediaSessions.insert(std::make_pair(mediaSession->getSessionName(), mediaSession));
    return true;
}

MediaSession* RtspServer::loopupMediaSession(std::string name)
{
    auto iter = mMediaSessions.find(name);
    if(iter == mMediaSessions.end())
        return NULL;
    return iter->second;
}

std::string RtspServer::getUrl(MediaSession* session)
{
    char url[200];
    snprintf(url, sizeof(url), "rtsp://%s:%d/%s", 
            sockets::getLocalIp().c_str(), mAddr.getPort(), session->getSessionName().c_str());
    return url;
}
