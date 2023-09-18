#ifndef RTSPSERVER_H
#define RTSPSERVER_H
#include <map>
#include <vector>
#include <string>

#include "TcpServer.h"
#include "UsageEnvironment.h"
#include "RtspConnection.h"
#include "MediaSession.h"
#include "Event.h"
#include "base/Mutex.h"

class RtspConnection;

class RtspServer : public TcpServer
{
public:
    static RtspServer* createNew(UsageEnvironment* env, Ipv4Address addr);

    RtspServer(UsageEnvironment* env, const Ipv4Address& addr);
    virtual ~RtspServer();

    UsageEnvironment* getEnv() const { return mEnv; }
    bool addMediaSession(MediaSession* mediaSession);
    MediaSession* loopupMediaSession(std::string name);
    std::string getUrl(MediaSession* session);

protected:
    virtual void handleNewConnection(int connFd);
    static void disconnectionCallback(void* arg, int sockFd);
    void handleDisconnection(int sockFd);
    static void triggerCallback(void*);
    void handleDisconnectionList();

private:
    std::map<std::string, MediaSession*> mMediaSessions;
    std::map<int, RtspConnection*> mConnections;
    std::vector<int> mDisconnectionList;
    TriggerEvent* mTriggerEvent;
    Mutex* mMutex;
};

#endif